#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "shader_s.h"
#include "camera.h"
#include "mc_table_gen.cpp"
#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>
#include "noise.hpp"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void getVertexCoordinates(int xIndex, int yIndex, int zIndex, float arr[8][3]);


const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const float X_MIN = -5.0f;
const float X_MAX = 5.0f;
const float Y_MIN = -1.0f;
const float Y_MAX = 1.0f;
const float Z_MIN = -5.0f;
const float Z_MAX = 5.0f;

const float UNIT_SIZE = 1.0f / 16.0f;


std::vector<float> vertices;

glm::vec4 cubePositions[8] = {
	//bottom vertices
	glm::vec4(-0.5f, -0.5f, 0.5f, 1.0f),
	glm::vec4(0.5f, -0.5f, 0.5f, 1.0f),
	glm::vec4(0.5f, -0.5f, -0.5f, 1.0f),
	glm::vec4(-0.5f, -0.5f, -0.5f, 1.0f),
	//Top vertices
	glm::vec4(-0.5f, 0.5f, 0.5f, 1.0f),
	glm::vec4(0.5f, 0.5f, 0.5f, 1.0f),
	glm::vec4(0.5f, 0.5f, -0.5f, 1.0f),
	glm::vec4(-0.5f, 0.5f, -0.5f, 1.0f),
};

// Abstract to an object so we can have other shapes
DataMat noise1 = gen_3d_noise(16);
DataMat noise2 = gen_3d_noise(16);
DataMat noise3 = gen_3d_noise(16);
DataMat noise4 = gen_3d_noise(16);

GradNoise gradNoise;

float density_fn(glm::vec3 vertex)
{
	float density = -vertex.y;
  density += gradNoise.sample(vertex);

	return density ;
}

float plane_fn(glm::vec3 vertex) {
	return -1.0f - vertex.y;
}

float sdfRecursiceTetra(glm::vec3 vertex) {
	glm::vec3 z = vertex;
	glm::vec3 a1 = glm::vec3(1,1,1);
	glm::vec3 a2 = glm::vec3(-1,-1,1);
	glm::vec3 a3 = glm::vec3(1,-1,-1);
	glm::vec3 a4 = glm::vec3(-1,1,-1);
	glm::vec3 c = a1;
	float scale = 4.0f;

	int n = 0;
	float dist, d;

	while (n < 5) {
		c = a1;
		dist = glm::length(z - a1);
		d = glm::length(z-a2); if (d < dist) { c = a2; dist=d; }
		d = glm::length(z-a3); if (d < dist) { c = a3; dist=d; }
		d = glm::length(z-a4); if (d < dist) { c = a4; dist=d; }
		z = scale*z-((scale-1.0f)*c);
		n++;
	}

	return 0.1f - (glm::length(z) * pow(scale, float(-n)));
}

float distanceToRecurringSpheres(glm::vec3 vertex) {

	return 0.3f - glm::length(glm::vec3(glm::mod(vertex.x, 1.0f) - 0.5, vertex.y, glm::mod(vertex.z, 1.0f) - 0.5));    

};

float distanceToSphere(glm::vec3 vertex) {
	return 0.5f - glm::length(vertex);
};

float getSphereVal(glm::vec3 p) {
	return 1;
}

glm::vec3 getNormal(glm::vec3 p, float (*f)(glm::vec3)) {
    const glm::vec2 k = glm::vec2(1.0f,-1.0f);
	return (k.xyy()*f(p + k.xyy()*UNIT_SIZE)+
						k.yyx()*f(p + k.yyx()*UNIT_SIZE) +
						k.yxy()*f(p + k.yxy()*UNIT_SIZE) +
						k.xxx()*f(p + k.xxx()*UNIT_SIZE));
}


//Unoptimized and glitchy, can create spheres-like object with the current configs
GLuint createSphereBuffer(float (*f)(glm::vec3)) {

	glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(UNIT_SIZE));
	float corner_values[8];
	for(float x=X_MIN; x<=X_MAX; x+=UNIT_SIZE) {
		for(float y=Y_MIN; y<=Y_MAX; y+=UNIT_SIZE) {
			for(float z=Z_MIN; z<=Z_MAX; z+=UNIT_SIZE) {
				uint8_t code = 0;
				uint8_t curr_vertex = 1;
				glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(x,y,z));
				for (int i = 0; i < 8; i++) {
					glm::vec4 cvertex = cubePositions[i];
					glm::vec4 newPos = translate * scale * cvertex;
					float density = f(newPos);
					corner_values[i] = density;

					if (density < 0.0f) {
						code |= curr_vertex;
					}
					curr_vertex = curr_vertex << 1;
				}
				glm::mat4 transform = translate*scale;
				std::vector<glm::vec3> unitVertices = getInterpolatedVertices(code, corner_values);
				if (code != 0) {
					for (int i=0; i < unitVertices.size(); i+=3) {
						glm::vec4 v1 = transform*glm::vec4(unitVertices[i], 1.0f);
						glm::vec4 v2 = transform*glm::vec4(unitVertices[i+1], 1.0f);
						glm::vec4 v3 = transform*glm::vec4(unitVertices[i+2], 1.0f);

						glm::vec4 tri_v1 = transform*glm::vec4(unitVertices[i], 1.0f);
						glm::vec4 tri_v2 = transform*glm::vec4(unitVertices[i+1], 1.0f);
						glm::vec4 tri_v3 = transform*glm::vec4(unitVertices[i+2], 1.0f);

						glm::vec2 uv1 = glm::vec2(tri_v1.x-X_MIN, tri_v1.z-Z_MIN);
						glm::vec2 uv2 = glm::vec2(tri_v2.x-X_MIN, tri_v2.z-Z_MIN);
						glm::vec2 uv3 = glm::vec2(tri_v3.x-X_MIN, tri_v3.z-Z_MIN);


						glm::vec3 norm1 = getNormal(tri_v1, f);
						glm::vec3 norm2 = getNormal(tri_v2, f);
						glm::vec3 norm3 = getNormal(tri_v3, f);

						glm::vec3 e1 = v2 - v1;
						glm::vec3 e2 = v3 - v2;

						glm::vec2 duv1 = uv2 - uv1;
						glm::vec2 duv2 = uv3 - uv2;

						float det = 1.0f / (uv1.x*uv2.y - uv1.y*uv2.x);

						glm::vec3 tangent;

						tangent.x = e1.x*uv2.y - e1.x*uv1.y;
						tangent.y = e1.y*uv2.y - e1.y*uv1.y;
						tangent.z = e1.z*uv2.y - e1.z*uv1.y;

						vertices.push_back(tri_v1.x);
						vertices.push_back(tri_v1.y);
						vertices.push_back(tri_v1.z);
						
						vertices.push_back(norm1.x);
						vertices.push_back(norm1.y);
						vertices.push_back(norm1.z);

						vertices.push_back(tri_v1.x);
						vertices.push_back(tri_v1.z);

						vertices.push_back(tangent.x);
						vertices.push_back(tangent.y);
						vertices.push_back(tangent.z);

						vertices.push_back(tri_v2.x);
						vertices.push_back(tri_v2.y);
						vertices.push_back(tri_v2.z);
						
						vertices.push_back(norm2.x);
						vertices.push_back(norm2.y);
						vertices.push_back(norm2.z);

						vertices.push_back(tri_v2.x);
						vertices.push_back(tri_v2.z);

						vertices.push_back(tangent.x);
						vertices.push_back(tangent.y);
						vertices.push_back(tangent.z);

						vertices.push_back(tri_v3.x);
						vertices.push_back(tri_v3.y);
						vertices.push_back(tri_v3.z);
						
						vertices.push_back(norm3.x);
						vertices.push_back(norm3.y);
						vertices.push_back(norm3.z);

						vertices.push_back(tri_v3.x);
						vertices.push_back(tri_v3.z);

						vertices.push_back(tangent.x);
						vertices.push_back(tangent.y);
						vertices.push_back(tangent.z);
					}
				}
			}
		}		
	}

	unsigned int VBO; 
    unsigned int VAO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11*sizeof(float), (void*)(sizeof(float)*3));
    glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11*sizeof(float), (void*)(sizeof(float)*6));
    glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11*sizeof(float), (void*)(sizeof(float)*8));
    glEnableVertexAttribArray(2);

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

int main() {
	// replace with time if needed
	srand(123);
	init_grad_vecs(123);
	
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

	mVAO = createSphereBuffer(distanceToSphere);


	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	int width, height, nrChannels;
	unsigned char *data = stbi_load("sand_col.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
    	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    	glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
    	std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	unsigned int texture2;
	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	data = stbi_load("sand_norm.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
    	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    	glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
    	std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glEnable(GL_DEPTH_TEST);

	Shader shader("vertex.vs", "fragment.fs");
	shader.use();
	shader.setInt("ourTexture", 0);

	while(!glfwWindowShouldClose(window)) {

		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		
		processInput(window);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH/(float)SCR_HEIGHT, 0.1f, 100.0f);
		shader.setMat4("projection", projection);
		
		glm::mat4 view = camera.GetViewMatrix();
		shader.setMat4("view", view);

		shader.setVec3("viewPos", camera.Position);

		glm::vec3 lightDir = glm::vec3(1.0f,1.0f,0.0f);
		shader.setVec3("lightDir", lightDir);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2);

		shader.setInt("ourTexture2", 1);
		
		
        shader.use();
		glBindVertexArray(mVAO);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size()/8);
		
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
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) 
	{
		camera.ProcessKeyboard(UP, deltaTime);
	}
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
