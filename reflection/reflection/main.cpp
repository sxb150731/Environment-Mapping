#define GLEW_STATIC
#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include <SOIL.h>

#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <cstdlib>

#include "shader.h"
#include "camera.h"
#include "texture.h"
#include "model.h"
#include "skybox.h"

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_move_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void do_movement();

// Properties
const int WINDOW_WIDTH = 1920, WINDOW_HEIGHT = 1080;

GLfloat lastX = WINDOW_WIDTH / 2.0f, lastY = WINDOW_HEIGHT / 2.0f;
bool firstMouseMove = true;
bool keyPressedStatus[1024]; 
GLfloat deltaTime = 0.0f; 
GLfloat lastFrame = 0.0f; 
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

int main(int argc, char** argv)
{

	if (!glfwInit())	
	{
		std::cout << "Error::GLFW could not initialize GLFW!" << std::endl;
		return -1;
	}

	// OpenGL 3.3 core profile
	std::cout << "Start OpenGL core profile version 3.3" << std::endl;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
		"Enviroment Mapping - reflection", NULL, NULL);
	if (!window)
	{
		std::cout << "Error::GLFW could not create winddow!" << std::endl;
		glfwTerminate();
		std::system("pause");
		return -1;
	}

	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_move_callback);
	glfwSetScrollCallback(window, mouse_scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glewExperimental = GL_TRUE; 
	GLenum status = glewInit();
	if (status != GLEW_OK)
	{
		std::cout << "Error::GLEW glew version:" << glewGetString(GLEW_VERSION) << " error string:" << glewGetErrorString(status) << std::endl;
		glfwTerminate();
		std::system("pause");
		return -1;
	}

	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	//Section1
	Model objModel;
	if (!objModel.loadModel("sphere.obj"))
	{
		glfwTerminate();
		std::system("pause");
		return -1;
	}
	// Section2
	std::vector<const char*> faces;
	faces.push_back("sky_rt.jpg");
	faces.push_back("sky_lf.jpg");
	faces.push_back("sky_up.jpg");
	faces.push_back("sky_dn.jpg");
	faces.push_back("sky_bk.jpg");
	faces.push_back("sky_ft.jpg");
	SkyBox skybox;
	skybox.init(faces);

	// Section3 
	Shader shader("scene.vertex", "scene.frag");
	Shader skyBoxShader("skybox.vertex", "skybox.frag");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDepthFunc(GL_LESS);

	while (!glfwWindowShouldClose(window))
	{
		GLfloat currentFrame = (GLfloat)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		glfwPollEvents(); 
		do_movement(); 

		 // colorBuffer
		glClearColor(0.18f, 0.04f, 0.14f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Draw scene as normal
		shader.use();
		glm::mat4 projection = glm::perspective(camera.mouse_zoom, (GLfloat)(WINDOW_WIDTH) / WINDOW_HEIGHT, 0.1f, 100.0f); 
		glUniformMatrix4fv(glGetUniformLocation(shader.programId, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glm::mat4 view = camera.getViewMatrix(); 
		glUniformMatrix4fv(glGetUniformLocation(shader.programId, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glm::mat4 model;
		glUniformMatrix4fv(glGetUniformLocation(shader.programId, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.getTextId());
		glUniform1i(glGetUniformLocation(shader.programId, "envText"), 0);
		glUniform3f(glGetUniformLocation(shader.programId, "cameraPos"), camera.position.x, camera.position.y, camera.position.z); 
		objModel.draw(shader);	

		skyBoxShader.use();
		view = glm::mat4(glm::mat3(camera.getViewMatrix())); 
		glUniformMatrix4fv(glGetUniformLocation(skyBoxShader.programId, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(skyBoxShader.programId, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.getTextId());
		glUniform1i(glGetUniformLocation(skyBoxShader.programId, "skybox"), 0);
		skybox.draw(skyBoxShader);

		glBindVertexArray(0);
		glUseProgram(0);
		glfwSwapBuffers(window); 
	}

	glfwTerminate();
	return 0;
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keyPressedStatus[key] = true;
		else if (action == GLFW_RELEASE)
			keyPressedStatus[key] = false;
	}
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE); 
	}
}
void mouse_move_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouseMove) 
	{
		lastX = xpos;
		lastY = ypos;
		firstMouseMove = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.handleMouseMove(xoffset, yoffset);
}

void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.handleMouseScroll(yoffset);
}

void do_movement()
{

	if (keyPressedStatus[GLFW_KEY_W])
		camera.handleKeyPress(FORWARD, deltaTime);
	if (keyPressedStatus[GLFW_KEY_S])
		camera.handleKeyPress(BACKWARD, deltaTime);
	if (keyPressedStatus[GLFW_KEY_A])
		camera.handleKeyPress(LEFT, deltaTime);
	if (keyPressedStatus[GLFW_KEY_D])
		camera.handleKeyPress(RIGHT, deltaTime);
	if (keyPressedStatus[GLFW_KEY_O])
		camera.handleKeyPress(UP, deltaTime);
	if (keyPressedStatus[GLFW_KEY_P])
		camera.handleKeyPress(DOWN, deltaTime);
}