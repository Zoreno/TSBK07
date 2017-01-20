#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

std::string getStringFromFile(const std::string& path)
{
	std::ifstream fileStream(path);
	return std::string{ std::istreambuf_iterator<char>(fileStream), std::istreambuf_iterator<char>() };
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main()
{
	//============================================================================
	// Init
	//============================================================================

	if(!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW!" << std::endl;
		return -1;
	}
	
	glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL 

	// Open a window and create its OpenGL context
	GLFWwindow* window;
	window = glfwCreateWindow(1024, 768, "Tutorial 01", NULL, NULL);
	if (window == nullptr) {
		std::cerr << "Failed to open GLFW window!" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window); // Initialize GLEW
	glewExperimental = true; // Needed in core profile
	if (glewInit() != GLEW_OK) {
		std::cerr << "Failed to initialize GLEW!" << std::endl;
		glfwTerminate();
		return -1;
	}

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	glViewport(0, 0, width, height);

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	//============================================================================
	// Vertex buffer setup
	//============================================================================

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// An array of 3 vectors which represents 3 vertices
	static const GLfloat g_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		0.0f,  1.0f, 0.0f,
	};

	// This will identify our vertex buffer
	GLuint vertexbuffer;
	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexbuffer);
	// The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	//============================================================================
	// Vertex Shader
	//============================================================================

	// Vertex shader ID
	GLuint vertexShader;

	// Create a shader object and save the ID
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	
	// Get shader source code and put it in a string
	std::string vertexShaderString = getStringFromFile("test.vert");
	const char* vertexShaderSource = vertexShaderString.c_str();

	// Associate code with shader object and compile.
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	// Get compilation status
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if(!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cerr << "ERROR: VERTEX SHADER COMPILATION FAULT: " << std::endl << infoLog << std::endl;
		glfwTerminate();
		return -1;
	}

	//============================================================================
	// Fragment Shader
	//============================================================================

	typedef GLuint FragmentShader;

	// Fragment Shader ID
	FragmentShader fragmentShader;

	// Create a shader object and save ID
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Get shader source code and put it in a string
	std::string fragmentShaderString = getStringFromFile("test.frag");
	const char* fragmentShaderSource = fragmentShaderString.c_str();

	// Associate code with shader object and compile.
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	// Get compilation status
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cerr << "ERROR: FRAGMENT SHADER COMPILATION FAULT: " << std::endl << infoLog << std::endl;
		glfwTerminate();
		return -1;
	}

	//============================================================================
	// Create and link shader program
	//============================================================================
	
	typedef GLuint ShaderProgram;

	ShaderProgram shaderProgram;
	shaderProgram = glCreateProgram();

	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cerr << "ERROR: SHADER PROGRAM LINKING FAULT: " << std::endl << infoLog << std::endl;
		glfwTerminate();
		return -1;
	}

	// Use our new shader program
	glUseProgram(shaderProgram);

	// Cleanup
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	//============================================================================
	// Static Uniforms
	//============================================================================

	//============================================================================
	// Register Callbacks
	//============================================================================

	glfwSetKeyCallback(window, key_callback);

	//============================================================================
	// Final Setup
	//============================================================================

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

	GLfloat time = (GLfloat)glfwGetTime();
	GLfloat timeElapsed = 0.f;
	GLuint frames = 0;

	while(glfwWindowShouldClose(window) == 0)
	{
		//============================================================================
		// Dynamic Uniforms
		//============================================================================

		// Recalculate time and FPS counter

		GLfloat oldTime = time;
		time = (GLfloat)glfwGetTime();
		timeElapsed += (time - oldTime);
		++frames;

		if(timeElapsed > 1.f)
		{
			timeElapsed -= 1.f;
			std::string newTitle = std::to_string(frames) + std::string{ " FPS" };
			glfwSetWindowTitle(window, newTitle.c_str());
			frames = 0;
		}

		// Transformation matrix. Used by vertex shader.
		mat4 trans;
		trans = glm::rotate(trans, radians(time*50.f), glm::vec3(0.0, 0.0, 1.0));
		trans = glm::scale(trans, glm::vec3(0.5f, 0.5f, 0.5f));
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "transform"), 1, GL_TRUE, value_ptr(trans));

		// Time value. Used by fragment shader.
		glUniform1f(glGetUniformLocation(shaderProgram, "time"), time);

		// Clear screen
		glClear(GL_COLOR_BUFFER_BIT);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			3* sizeof(GLfloat),                  // stride
			(GLvoid*)0            // array buffer offset
			);
		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
		glDisableVertexAttribArray(0);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	glfwTerminate();

	return 0;
}