#include <iostream>

#define GLEW_STATIC

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

glm::mat4 modelMatrix = (1.f);

const GLint WIDTH = 800, HEIGHT = 600;

int main()
{
	//windowの初期化
	if (!glfwInit())
	{
		std::cout << "GLFW is faild Initialize!" << "/n";
		glfwTerminate();
		return 1;
	}

	//Setup Window Properties 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	

	GLFWwindow* const window(glfwCreateWindow(640, 480, "Hello", NULL, NULL));

	if (!window)
	{
		std::cout << "Failed to create window!" << "/n";
		glfwTerminate();
		return 1;
	}

	//Get frame buffer size
	int bufferWidth, bufferHeight;
	glfwGetFramebufferSize(window, &bufferWidth, &bufferHeight);

	//Set context
	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;

	//glewの初期化
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW!" << "\n";
		glfwDestroyWindow(window);
		glfwTerminate();
		return 1;
	}

	//Viewport
	glViewport(0, 0, bufferWidth, bufferHeight);

	//Main loop
	while (!glfwWindowShouldClose(window))
	{
		//Get and handle user inputs
		glfwPollEvents();

		//Render
		//Clear buffers
		glClearColor(1.f, 0.f, 0.f, 255.f);
		glClear(GL_COLOR_BUFFER_BIT);

		glfwSwapBuffers(window);
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;

	return 0;
}

//プログラムオブジェクトを作成する
//  vsrc:バーテックシェーダーのソースプログラムの文字列
//  vsrc:フラグメントシェーダーのソースプログラムの文字列
GLuint createProgram(const char* vsrc, const char* fsrc)
{
	//空のオブジェクトを作成する
	const GLuint program(glCreateProgram());

	if (vsrc != NULL)
	{
		//バーテックスシェーダーのシェイダーオブジェクトを作成
		const GLuint vobj(glCreateShader(GL_VERTEX_SHADER));
		glShaderSource(vobj, 1, &vsrc, NULL);
		glCompileShader(vobj);

		//バーテックスシェーダーオブジェクトをプロジェクトオブジェクトに読み込む
		glAttachShader(program, vobj);
		glDeleteShader(vobj);
	}

	if (fsrc != NULL)
	{
		//フラグメントシェーダーのシェーダーオブジェクトを作成する
		const GLuint fobj(glCreateShader(GL_FRAGMENT_SHADER));
		glShaderSource(fobj, 1, &fsrc, NULL);
		glCompileShader(fobj);

		//フラグメントシェーダーのシェーダーオブジェクトをプログラムオブジェクトに組み込む
		glAttachShader(program, fobj);
		glDeleteShader(fobj);
	}

	//プログラムオブジェクトをリンクする
	glBindAttribLocation(program, 0, "position");
	glBindFragDataLocation(program, 0, "fragment");
	glLinkProgram(program);

	//作成したプログラムオブジェクトを返す
	return program;
}

