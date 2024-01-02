#pragma once
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
#include "./buffer.h"
struct ShaderProgramSource {
	std::string VertexShader;
	std::string FragmentShader;
};
static ShaderProgramSource ParseShader(const std::string filePath) {



	std::ifstream stream(filePath);
	if (!stream.is_open()) {
		std::cout << "Failed to open the file: " + filePath << std::endl;
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
		if (line.find("#shader") != std::string::npos) {
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

	return { sstr[0].str(), sstr[1].str() };
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

			luupinnng();

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