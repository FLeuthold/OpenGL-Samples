#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>
#include <iostream>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

float r2 = 0.0f;
float increment2 = 0.05f;
float r = 0.0f;
float increment = 0.05f;
GLuint program;
GLuint vao;
GLuint buffer;
GLuint buffer1;
unsigned char* ptr1;
struct vertex {
	// Position
	float x;
	float y;
	float z;
	float w;

	// Color
	float r;
	float g;
	float b;
	float a;
};

static void beiexit() {

	
	glDeleteVertexArrays(1, &vao);
	glDeleteProgram(program);
	glDeleteBuffers(1, &buffer);
	glDeleteBuffers(1, &buffer1);
	//glUnmapBuffer(GL_UNIFORM_BUFFER); 
}


static void init() {



	static const vertex vertices[] = {
		{ 0.25f, -0.25f, 0.5f, 1.0f ,
		  1.00f,  0.00f, 0.0f, 1.0f },

		{ -0.25f, -0.25f, 0.5f, 1.0f,
		   0.00f,  1.00f, 0.0f, 1.0f },

		{ 0.25f,  0.25f, 0.5f, 1.0f,
		  0.00f,  0.00f, 1.0f, 1.0f }
	};

	GLenum err;
	//glGenVertexArrays(1, &vao);
	glCreateVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glCreateBuffers(1, &buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer);
	//allocate space on gpu ram
	glNamedBufferStorage(buffer, sizeof(vertices), vertices, 0);
	

	//Position
	glVertexArrayAttribFormat(vao, 0, 4, GL_FLOAT, GL_FALSE, offsetof(vertex, x));
	glVertexArrayAttribBinding(vao, 0, 0);
	glEnableVertexArrayAttrib(vao, 0);

	//Color
	glVertexArrayAttribFormat(vao, 1, 4, GL_FLOAT, GL_FALSE, offsetof(vertex, r));
	glVertexArrayAttribBinding(vao, 1, 0);
	glEnableVertexArrayAttrib(vao, 1);

	glVertexArrayVertexBuffer(vao, 0, buffer, 0, sizeof(vertex));

	/////////////////////////////////
	//Uniform Block

	glm::mat4 transl_matrix;
	glm::mat4 rot_matrix;
	glm::mat4 proj_matrix;

	transl_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.5f));
	rot_matrix = glm::rotate(glm::mat4(1.0f), .5f, glm::vec3(0.0f, 0.0f, 1.0f));//angle is in radians
	proj_matrix = glm::perspective(0.5f, 1.5f, 0.5f, 10.0f);//fovy is in radians, not degrees

	//buffer for transformation matrices
	
	glCreateBuffers(1, &buffer1);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, buffer1);// for INDEX  "1" see figure 5.1 at Superbible page 133
	glNamedBufferStorage(buffer1, 4096, NULL, GL_MAP_WRITE_BIT | GL_DYNAMIC_STORAGE_BIT);
	
	//Assuming you have matrices defined as GLfloat arrays
	//glNamedBufferSubData(buffer1, 0, sizeof(glm::mat4), glm::value_ptr(transl_matrix));
	//glNamedBufferSubData(buffer1, 80, sizeof(glm::mat4), glm::value_ptr(rot_matrix));
	//glNamedBufferSubData(buffer1, 160, sizeof(glm::mat4), glm::value_ptr(proj_matrix));

	ptr1 = (unsigned char*)glMapNamedBuffer(buffer1, GL_WRITE_ONLY);
	
	//glBufferStorage(GL_UNIFORM_BUFFER, 4096, NULL, GL_MAP_WRITE_BIT);
	//GLuint buffer2 = (unsigned char*)malloc(4096);
	//ptr1 = (unsigned char*)glMapNamedBuffer(buffer1, GL_WRITE_ONLY);//glMapBuffer returns void pointer
	//glBindBuffer(GL_UNIFORM_BUFFER, buffer1);
	//glBufferStorage(GL_UNIFORM_BUFFER, sizeof(float) * 16 * 3, nullptr, GL_MAP_WRITE_BIT);

	
	// ptr1 = (unsigned char*)glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	
	
	
	
	
	if (ptr1 != NULL) {

		for (int i = 0; i < 4; ++i) {
			GLuint offset1 = 0 + 16 * i;//translation
			GLuint offset2 = 80 + 16 * i;//rotation
			GLuint offset3 = 160 + 16 * i;//projection
			for (int j = 0; j < 4; ++j) {
				*((float*)(ptr1 + offset1)) = transl_matrix[i][j];// adding char and int together
				*((float*)(ptr1 + offset2)) = rot_matrix[i][j];
				*((float*)(ptr1 + offset3)) = proj_matrix[i][j];
				offset1 += 4;
				offset2 += 4;
				offset3 += 4;
			}
		}

	}

	glUnmapNamedBuffer(buffer1);
	//glUnmapBuffer(GL_UNIFORM_BUFFER);


	

}









struct ShaderProgramSource {
	std::string VertexShader;
	std::string FragmentShader;
};
static ShaderProgramSource ParseShader(const std::string filePath) {


	
	std::ifstream stream(filePath);
	if (!stream.is_open()) {
		std::cout<<"Failed to open the file: " + filePath << std::endl;
	}
	std::string line;
	std::stringstream sstr[2];
	enum class ShaderType {
		NONE = -1,
		VERTEX = 0,
		FRAGMENT = 1
	};
	ShaderType type = ShaderType::NONE;
	while (getline(stream, line)) {
		if (line.find("#shader")!= std::string::npos) {
			if (line.find("vertex") != std::string::npos) {
				type = ShaderType::VERTEX;
			}
			else if (line.find("fragment") != std::string::npos) {
				type = ShaderType::FRAGMENT;
			}
		}
		else {
			sstr[(int)type] << line << '\n';
		}
	}

	return {sstr[0].str(), sstr[1].str()};
}

static void GLAPIENTRY errorOccurredGL(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	

	if (type == GL_DEBUG_TYPE_ERROR) {
		fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
			(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
			type, severity, message);
		
		exit(-1); 
	
	}

}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;


	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
	glfwSwapInterval(5);




    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        std::cout << "Error:" << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Status: Using GLEW" << glewGetString(GLEW_VERSION) << std::endl;
    std::cout << "Status: Using OpenGL" << glGetString(GL_VERSION) << std::endl;

	//glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(errorOccurredGL, 0);
	
    {

		
		ShaderProgramSource source = ParseShader("Block.shader");

		static const char* vs_source = source.VertexShader.c_str();

		static const char* fs_source = source.FragmentShader.c_str();

		program = glCreateProgram();
		GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fs, 1, &fs_source, NULL);
		glCompileShader(fs);

		GLuint vs = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vs, 1, &vs_source, NULL);
		glCompileShader(vs);

		glAttachShader(program, vs);
		glAttachShader(program, fs);

		glLinkProgram(program);

		init();

        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {


			/* Render here */
			glClear(GL_COLOR_BUFFER_BIT);


			// Some computation here
			if (r2 > 1.0f || r2 < 0.0f) {
				increment2 *= -1.0f;
			}
			r2 += increment2;


			r += increment;
			//glm::mat4x4 rot_matrix = glm::rotate(glm::mat4(1.0f), r, glm::vec3(0.0f, 0.0f, 1.0f));
			//const GLfloat color[] = { r , r , 0.0f, 1.0f};
			const GLfloat color[] = { 1.0f - r2 , r2 , 0.0f, 1.0f };
			glClearBufferfv(GL_COLOR, 0, color);

			glm::mat4 rot_matrix = glm::rotate(glm::mat4(1.0f), r , glm::vec3(0.0f, 0.0f, 1.0f));//angle is in radians
			glNamedBufferSubData(buffer1, 80, sizeof(glm::mat4), glm::value_ptr(rot_matrix));
			/*if (ptr1 != NULL) {

				for (int i = 0; i < 4; ++i) {
					GLuint offset2 = 80 + 16 * i;//rotation
					for (int j = 0; j < 4; ++j) {
						*((float*)(ptr1 + offset2)) = rot_matrix[i][j];
					}
				}

			}*/
			glUseProgram(program);
			glDrawArrays(GL_TRIANGLES, 0, 3);

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            /* Poll for and process events */
            glfwPollEvents();
        }
        //delete here


		beiexit();

		
    }
    glfwTerminate();
    return 0;
}





