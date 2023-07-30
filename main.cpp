#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "shader_s.h"
#include "camera.h"
#include "mc_table_gen.cpp"
#include <vector>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void getVertexCoordinates(int xIndex, int yIndex, int zIndex, float arr[8][3]);


const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const float RESOLUTION = 100.0f;
const float UNIT_SIZE = 0.01f;

const unsigned int NUM_BOX_X = (unsigned int) RESOLUTION;
const unsigned int NUM_BOX_Y = (unsigned int) RESOLUTION;
const unsigned int NUM_BOX_Z = (unsigned int) RESOLUTION;

std::vector<float> vertices;

glm::vec4 cubePositions[8] = {
	//bottom vertices
	glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f),
	glm::vec4(1.0f, -1.0f, -1.0f, 1.0f),
	glm::vec4(1.0f, -1.0f, 1.0f, 1.0f),
	glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f),
	//Top vertices
	glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f),
	glm::vec4(1.0f, 1.0f, -1.0f, 1.0f),
	glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
	glm::vec4(-1.0f, 1.0f, 1.0f, 1.0f),
};

// Abstract to an object so we can have other shapes
bool isInSphere(glm::vec3 vertex) {
	return glm::length(vertex) < 0.25f;
};


//Unoptimized and glitchy, can create spheres-like object with the current configs
GLuint createSphereBuffer() {
	glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(UNIT_SIZE));
	int item_count = 0;
	for(float x=-1.0f; x<1.0f; x+=UNIT_SIZE) {
		for(float y=-1.0f; y<1.0f; y+=UNIT_SIZE) {
			for(float z=-1.0f; z<1.0f; z+=UNIT_SIZE) {
				uint8_t code = 0;
				uint8_t curr_vertex = 1;
				glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(x,y,z));
				for (glm::vec4 cvertex: cubePositions) {
					glm::vec4 newPos = translate * scale * cvertex;
					if (isInSphere(glm::vec3(newPos.x, newPos.y, newPos.z))) {
						code |= curr_vertex;
					}
					curr_vertex = curr_vertex << 1;
				}
				std::vector<float> unitVertices = getVertices(code);
				if (code != 0) {
					for (int i=0; i < unitVertices.size(); i+=3) {
						glm::vec4 tri_v = translate*scale*glm::vec4(unitVertices[i], unitVertices[i+1], unitVertices[i+2], 1.0f);
						item_count += 3;
						vertices.push_back(tri_v.x);
						vertices.push_back(tri_v.y);
						vertices.push_back(tri_v.z);
					}

				}
			}
		}		
	}
	float vertexBufferData[vertices.size()];
	std::copy(vertices.begin(), vertices.end(), vertexBufferData);
	unsigned int VBO; 
    unsigned int VAO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexBufferData), vertexBufferData, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
	return VAO;
};

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;


float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

uint8_t config_num = 1;
std::vector<float> cubeVector(12);
GLuint mVAO;

GLuint createBuffers()
{
	//Creating array for marching cube triangulation to render
	cubeVector = getVertices(config_num);
	float vertexBufferData[cubeVector.size()];
	std::copy(cubeVector.begin(), cubeVector.end(), vertexBufferData);
	unsigned int VBO; 
    unsigned int VAO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexBufferData), vertexBufferData, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

	return VAO;
}

int main() {
	
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For Apple
	#endif
	
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Window", nullptr, nullptr);
	
	if (window == NULL) {
		std::cout << "Failed to create window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	gen_table();

	mVAO = createSphereBuffer();

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glEnable(GL_DEPTH_TEST);

	Shader shader("vertex.vs", "fragment.fs");
	shader.use();

	while(!glfwWindowShouldClose(window)) {

		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		
		processInput(window);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		shader.use();
		
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH/(float)SCR_HEIGHT, 0.1f, 100.0f);
		shader.setMat4("projection", projection);
		
		glm::mat4 view = camera.GetViewMatrix();
		shader.setMat4("view", view);
		
		glBindVertexArray(mVAO);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
uint32_t lastChangeTime = 0;
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && lastChangeTime == 0)
	{
		config_num += 1;
		mVAO = createBuffers();
		lastChangeTime = 15;
	}

	if (lastChangeTime) lastChangeTime -= 1;
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}
