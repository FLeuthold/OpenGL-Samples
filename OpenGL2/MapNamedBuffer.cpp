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


struct ShaderProgramSource {
	std::string VertexShader;
	std::string FragmentShader;
};
static ShaderProgramSource ParseShader(const std::string filePath) {
	std::ifstream stream(filePath);
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

void GLAPIENTRY errorOccurredGL(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	printf("Message from OpenGL:\nSource: 0x%x\nType: 0x%x\n"
		"Id: 0x%x\nSeverity: 0x%x\n", source, type, id, severity);
	printf("%s\n", message);
	exit(-1); // shut down the program gracefully (it does cleanup stuff too)
  // without exit(), OpenGL will constantly print the error message... make sure to kill your program.
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

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(errorOccurredGL, NULL);
	
    {
		GLuint          vao;
		GLuint          buffer;
		ShaderProgramSource source = ParseShader("Block.shader");

		static const char* vs_source = source.VertexShader.c_str();

		static const char* fs_source = source.FragmentShader.c_str();

		GLuint program = glCreateProgram();
		GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fs, 1, &fs_source, NULL);
		glCompileShader(fs);

		GLuint vs = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vs, 1, &vs_source, NULL);
		glCompileShader(vs);

		glAttachShader(program, vs);
		glAttachShader(program, fs);

		glLinkProgram(program);

		//////////////////////////

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
		static const vertex vertices[] = {
			{ 0.25f, -0.25f, 0.5f, 1.0f ,
			  1.00f,  0.00f, 0.0f, 1.0f },

			{ -0.25f, -0.25f, 0.5f, 1.0f,
			   0.00f,  1.00f, 0.0f, 1.0f },

			{ 0.25f,  0.25f, 0.5f, 1.0f,
			  0.00f,  0.00f, 1.0f, 1.0f }
		};
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);


		glCreateBuffers(1, &buffer);
		glNamedBufferStorage(buffer, sizeof(vertices), NULL, GL_MAP_WRITE_BIT);//GL_DYNAMIC_STORAGE_BIT
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer);

		void* ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
		memcpy(ptr, vertices, sizeof(vertices));
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);//GL_ARRAY_BUFFER

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

		glm::mat4 transl_matrix = glm::translate(glm::mat4(1.0f), glm::vec3( 0.0f, 0.0f, -2.5f));
		glm::mat4 rot_matrix = glm::rotate(glm::mat4(1.0f), 30.0f, glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4 proj_matrix = glm::perspective(0.5f, 1.5f, 1.0f, 10.0f);//fovy is in radians, not degrees

		/*static const GLchar* uniformNames[3] = {
			"translation",
			"rotation",
			"projection_matrix"
		};*/
		//GLuint uniformIndices[3];
		//
		//glGetUniformIndices(program, 3, uniformNames, uniformIndices);

		//GLint uniformOffsets[3];
		//GLint matrixStrides[3];
		//glGetActiveUniformsiv(program, 3, uniformIndices, GL_UNIFORM_OFFSET, uniformOffsets);
		//glGetActiveUniformsiv(program, 3, uniformIndices, GL_UNIFORM_MATRIX_STRIDE, matrixStrides);
		//unsigned int block_index = glGetUniformBlockIndex(program, "TransformBlock");
		//glUniformBlockBinding(program, block_index, 0);
		//glBindBuffer(GL_UNIFORM_BUFFER, buffer1);

		//
		
		GLuint buffer1;
		glCreateBuffers(1, &buffer1);
		glBindBufferBase(GL_UNIFORM_BUFFER,1, buffer1);// for INDEX  "1" see figure 5.1 at Superbible page 133
		glNamedBufferStorage(buffer1, 4096, NULL, GL_MAP_WRITE_BIT);
		//glBufferStorage(GL_UNIFORM_BUFFER, 4096, NULL, GL_MAP_WRITE_BIT);


		//GLuint buffer2 = (unsigned char*)malloc(4096);
		unsigned char * ptr1 = (unsigned char*) glMapNamedBuffer(buffer1, GL_WRITE_ONLY);//glMapBuffer returns void pointer
		//unsigned char* ptr1 = (unsigned char*)glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
		//
		//

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
		glUnmapBuffer(GL_UNIFORM_BUFFER);
		
		float r = 0.0f;
		float increment = 0.05f;



        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {
            /* Render here */
            glClear(GL_COLOR_BUFFER_BIT);

			
			// Some computation here
			if (r > 1.0f) {
				increment = -0.05f;
			}
			else if (r < 0.0f) {
				increment = 0.05f;
			}

			r += increment;

			const GLfloat color[] = { 1.0f-r , r , 0.0f, 1.0f };
			glClearBufferfv(GL_COLOR, 0, color);
			glUseProgram(program);
			glDrawArrays(GL_TRIANGLES, 0, 3);

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            /* Poll for and process events */
            glfwPollEvents();
        }
        //delete here

		glDeleteVertexArrays(1, &vao);
		glDeleteProgram(program);
		glDeleteBuffers(1, &buffer);
		glDeleteBuffers(1, &buffer1);
		
    }
    glfwTerminate();
    return 0;
}





