#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader.h"
#include "transform_shader.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "camera.hpp"

float previousTime = 0.0;
float deltaTime = 1.0f / 60.0f;
double oldXPos = 0.0;
double oldYPos = 0.0;
bool firstMovement = true;

extern GLFWwindow* setupWindow(int width, int height);
extern void setupGlad();
extern void handleInputs(GLFWwindow* window);
extern void handleMouseMovement(GLFWwindow* window, double xpos, double ypos);

const int WIDTH = 1000;
const int HEIGHT = 1000;

Camera* camera = new Camera(glm::vec3(0.0f, 0.0f, 70.0f));

struct FB {
    GLuint fbo;
    GLuint texture;
};

FB createFramebuffer()
{
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLuint densityTextureBuffer;
    glGenTextures(1, &densityTextureBuffer);
    glBindTexture(GL_TEXTURE_3D, densityTextureBuffer);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, 33, 33, 33, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_3D, 0);

    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return { fbo, densityTextureBuffer };
}

float chunkStepSize = 0.25f;
void generate_chunk_value(GLuint quadVao, glm::vec3 chunkPosition, FB texture3DFramebuffer, const Shader& text3DProgram)
{
    // Render to frame buffer the sphere
    glBindVertexArray(quadVao);
    glViewport(0, 0, 33, 33);
    glBindFramebuffer(GL_FRAMEBUFFER, texture3DFramebuffer.fbo);
    glBindTexture(GL_TEXTURE_3D, texture3DFramebuffer.texture);
        
    for (int i = 0; i < 33; i++)
    {
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture3DFramebuffer.texture, 0, i);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cout << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
            throw std::runtime_error("Framebuffer not complete");
        }

        glEnable(GL_DEPTH_TEST);
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        text3DProgram.use();
        text3DProgram.setVec3("chunkPosition", (float*) glm::value_ptr(chunkPosition));
        text3DProgram.setFloat("chunkWidth", 32.0f);
        text3DProgram.setFloat("wZ", static_cast<float>(i));
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}

int main()
{
    const glm::vec3 lightDir(1.0, 1.0, 1.0);

    GLFWwindow* window = setupWindow(WIDTH, HEIGHT);
    setupGlad();

    glm::mat4 perspectiveTransform = glm::perspective(glm::radians(45.0), (double) WIDTH / HEIGHT, 0.1, 10000.0);

    Shader finalRenderShader("./shaders/basic.vs", "./shaders/basic.fs");


    FB renderImageFramebuffer = createFramebuffer();
    Shader postRenderProgram("./shaders/post.vs", "./shaders/post.fs");
    float quadVertices[24] {
        -1.0, -1.0, 0.0, 0.0,
         1.0, -1.0, 1.0, 0.0,
         1.0,  1.0, 1.0, 1.0,

        -1.0, -1.0, 0.0, 0.0,
         1.0,  1.0, 1.0, 1.0,
        -1.0,  1.0, 0.0, 1.0,
    };

    GLuint quadVao;
    glGenVertexArrays(1, &quadVao);
    glBindVertexArray(quadVao);

    GLuint quadVertexBuffer;
    glGenBuffers(1, &quadVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, quadVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    postRenderProgram.use();
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*) (2 * sizeof(float)));
    glEnableVertexAttribArray(1);


    Shader text3DProgram("./shaders/text3D.vs", "./shaders/text3D.fs");
    text3DProgram.use();
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);

    const char* varyings[2] { "outVec", "outNormal" };
    TransformOutputParams params {
        .varyings = varyings,
        .varyingsCount = 2,
        .bufferMode = GL_INTERLEAVED_ATTRIBS 
    };
    TransformShader mcBufferShader("./shaders/mc.vs", "./shaders/mc.gs", params);
    // generate a cube buffer;
    std::vector<float> cubeVertices;
    for (int i = 0; i < 32; i++)
    {
        for (int j = 0; j < 32; j++)
        {
            for (int k = 0; k < 32; k++)
            {
                cubeVertices.push_back(static_cast<float>(i));
                cubeVertices.push_back(static_cast<float>(j));
                cubeVertices.push_back(static_cast<float>(k));
            }
        }
    }

    mcBufferShader.use();
    GLuint cubeVao;
    glGenVertexArrays(1, &cubeVao);
    glBindVertexArray(cubeVao);

    GLuint cubeBuffer;
    glGenBuffers(1, &cubeBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, cubeBuffer);
    glBufferData(GL_ARRAY_BUFFER, cubeVertices.size() * sizeof(float), cubeVertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);

    // to capture the feedback buffer
    GLuint feedbackObj;
    glGenTransformFeedbacks(1, &feedbackObj);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedbackObj);

    GLuint svao;
    glGenVertexArrays(1, &svao);
    glBindVertexArray(svao);

    GLuint mcOutputBuffer;
    glGenBuffers(1, &mcOutputBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mcOutputBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * cubeVertices.size() * 9 , nullptr, GL_STATIC_DRAW);
        
    long frameCount = 0;
    int layerIndex = 31;
    glm::vec3 chunkPosition = glm::vec3(-16.0f);
    generate_chunk_value(quadVao, chunkPosition, renderImageFramebuffer, text3DProgram);
    while (!glfwWindowShouldClose(window))
    {
        float currentTime = glfwGetTime();
        deltaTime = currentTime - previousTime;
        previousTime = currentTime;

        handleInputs(window);
        if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
        {
            layerIndex += 1;
            layerIndex %= 33;
        }
        glViewport(0, 0, WIDTH, HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        mcBufferShader.use();
        mcBufferShader.setVec3("uChunkPosition", glm::value_ptr(chunkPosition));
        mcBufferShader.setFloat("uChunkWidth", 32.0f);
        glDisable(GL_DEPTH_TEST);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, renderImageFramebuffer.texture);
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mcOutputBuffer); // 32*32*32*12*sizeof(float));
        glBindVertexArray(cubeVao);
        glBeginTransformFeedback(GL_TRIANGLES);
        glDrawArrays(GL_POINTS, 0, cubeVertices.size());
        glEndTransformFeedback();
        glUseProgram(0);

        finalRenderShader.use();
        glm::mat4 cameraTransfrom = camera->GetViewMatrix();
        finalRenderShader.setFloatMat4("uWorldTransform", (float*) glm::value_ptr(glm::mat4(1.0)));
        finalRenderShader.setFloatMat4("uCameraTransform", glm::value_ptr(cameraTransfrom));
        finalRenderShader.setFloatMat4("uPerspectiveTransform", glm::value_ptr(perspectiveTransform));
        finalRenderShader.setVec3("uLightDir", (float*) glm::value_ptr(lightDir));

        glBindVertexArray(svao); 
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) 0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) (3 * sizeof(float)));
        glDrawTransformFeedback(GL_TRIANGLES, feedbackObj);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return 0;
}

GLFWwindow* setupWindow(int width, int height)
{
    glfwInit();

    glfwWindowHint(GLFW_OPENGL_CORE_PROFILE, GL_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_VERSION_MINOR, 3);

    GLFWwindow* window = glfwCreateWindow(width, height, "Model loading tutorial", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Window cannot be created." << std::endl;
        exit(1);
    }

    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, handleMouseMovement);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    return window;
}

void setupGlad()
{
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        std::cerr << "Error setting up GLAD" << std::endl;
        exit(1);
    }
}

void handleInputs(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        camera->ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        camera->ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        camera->ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        camera->ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        camera->ProcessKeyboard(Camera_Movement::UP, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        camera->ProcessKeyboard(Camera_Movement::DOWN, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

}

void handleMouseMovement(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMovement)
    {
        oldXPos = xpos;
        oldYPos = ypos;
        firstMovement = false;
    }

    double xOffset = xpos - oldXPos;
    double yOffset = ypos - oldYPos;

    camera->ProcessMouseMovement(xOffset, yOffset);

    oldXPos = xpos;
    oldYPos = ypos;
}