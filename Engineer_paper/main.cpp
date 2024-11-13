
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <random>
#include <fstream>
#include <sstream>
#include <vector>


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void get_vertices(std::string line, std::vector<float> &vertices) {
	int occurences = 0;
	int index = 0;
	for (int i = 0; i < line.length(); i++) {
		if (line[i] == ' ') {
			if (occurences > 0) {
				vertices.push_back(stof(line.substr(index, i - index)));
			}
				
			index = i;
			occurences++;
		}
	}
}

float* import_3DObject(std::string filename) {
	std::ifstream import_file(filename);
	std::string line = "";
	std::vector<float> vertices;

	while (!import_file.eof()) {
		getline(import_file, line);
		if (line[0] == 'v') {
			get_vertices(line, vertices);
		}
	}
}

int main() {

	//----------INITIATE GLFW
	if (!glfwInit())
	{
		std::cout << "GLFW init failed" << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//----------INITIATE WINDOW
	GLFWwindow* window = glfwCreateWindow(1400, 787, "ProjektInzynierski", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	//----------INITIATE GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glViewport(0,0, 800, 600);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	float* color = new float[3]{ .0f,.0f,.0f };

	//---------RENDER LOOP
	while (!glfwWindowShouldClose(window))
	{
		//get inputs
		processInput(window);

		//render
		glClearColor(color[0], color[1], color[2], 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		for (int i = 0; i < 3; i++) {
			color[i] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		}


		//end

		glfwPollEvents();
		glfwSwapBuffers(window);		
	}

	delete[] color;

	glfwTerminate();
	return 0;
}
