#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader.h"
#include "transform_shader.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "camera.hpp"
#include <chrono>
#include <random>
#include "noise.hpp"
#include "texture.hpp"

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

Camera* camera = new Camera(glm::vec3(0.0f, 0.0f, 5.0f));

struct GlobalChunkConfig
{
    int numVoxelsPerDim;
    float voxelSize;
};

GlobalChunkConfig GLOBAL_CHUNK_CONFIG {
    32,
    10.0f
};

struct Texture3DBuffer {
    GLuint fbo;
    GLuint texture;
};

void print_vec(glm::vec3 p)
{
    std::cout << p.x << ", " << p.y << ", " << p.z << std::endl;
}

DataMat noise1 = gen_3d_noise(16);
DataMat noise2 = gen_3d_noise(16);
DataMat noise3 = gen_3d_noise(16);
DataMat noise4 = gen_3d_noise(16);
DataMat noise5 = gen_3d_noise(16);

void myfunc(int (* func)(int a, int b))
{
    std::cout << func(1, 2) << std::endl;
}

GLuint createNoiseTexture(DataMat noise)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_3D, texture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, 16, 16, 16, 0, GL_RED, GL_FLOAT, noise.get_backing_data());
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glBindTexture(GL_TEXTURE_3D, 0);
    return texture;
}

Texture3DBuffer createFramebuffer()
{
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLuint densityTextureBuffer;
    glGenTextures(1, &densityTextureBuffer);
    glBindTexture(GL_TEXTURE_3D, densityTextureBuffer);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, GLOBAL_CHUNK_CONFIG.numVoxelsPerDim + 1, GLOBAL_CHUNK_CONFIG.numVoxelsPerDim + 1, GLOBAL_CHUNK_CONFIG.numVoxelsPerDim + 1, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
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

void generate_chunk_value(GLuint quadVao, glm::vec3 chunkPosition, Texture3DBuffer texture3DFramebuffer, const Shader& text3DProgram)
{
    // Render to frame buffer the sphere
    glBindVertexArray(quadVao);
    glViewport(0, 0, GLOBAL_CHUNK_CONFIG.numVoxelsPerDim + 1, GLOBAL_CHUNK_CONFIG.numVoxelsPerDim + 1);
    glBindFramebuffer(GL_FRAMEBUFFER, texture3DFramebuffer.fbo);
    glBindTexture(GL_TEXTURE_3D, texture3DFramebuffer.texture);
        
    for (int i = 0; i < GLOBAL_CHUNK_CONFIG.numVoxelsPerDim + 1; i++)
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
        text3DProgram.setFloat("numVoxelPerDim", static_cast<float>(GLOBAL_CHUNK_CONFIG.numVoxelsPerDim));
        text3DProgram.setFloat("voxelSize", GLOBAL_CHUNK_CONFIG.voxelSize);
        text3DProgram.setFloat("planeZLocal", static_cast<float>(i) * GLOBAL_CHUNK_CONFIG.voxelSize);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}

struct ChunkMeshOutput
{
    GLuint vao;
    GLuint feedbackObj;
    GLuint bufferBase;
};

struct Chunk
{
    ChunkMeshOutput meshOutput;
    glm::vec3 position;
};

std::vector<glm::vec3> chunksToCreate;
std::vector<Chunk> createdChunks;

void initChunksToCreate()
{
    int BOUND_X_START = 0;
    int BOUND_X_END = 5 ;
    int BOUND_Y_START = 0;
    int BOUND_Y_END = 3 ;
    int BOUND_Z_START = 0 ;
    int BOUND_Z_END = 5;
    float worldChunkWidth = GLOBAL_CHUNK_CONFIG.numVoxelsPerDim * GLOBAL_CHUNK_CONFIG.voxelSize;
    for (int i = BOUND_X_START; i < BOUND_X_END; i++)
    {
        float x = ((float) i) * worldChunkWidth;
        for (int j = BOUND_Y_START; j < BOUND_Y_END; j++)
        {
            float y = ((float) j) * worldChunkWidth;
            for (int k = BOUND_Z_START; k < BOUND_Z_END; k++)
            {
                float z = ((float) k) * worldChunkWidth;
                chunksToCreate.push_back(glm::vec3(x, y, z));
            }
        }
    }

    std::cout << chunksToCreate.size() << std::endl;
}

struct NoiseTextures
{
    GLuint noise1;
    GLuint noise2;
    GLuint noise3;
    GLuint noise4;
};

ChunkMeshOutput generateChunkMesh(glm::vec3 chunkPosition, Texture3DBuffer renderImageFramebuffer, NoiseTextures noiseTextures)
{
    const char* varyings[2] { "outVec", "outNormal" };
    TransformOutputParams params {
    .varyings = varyings,
    .varyingsCount = 2,
    .bufferMode = GL_INTERLEAVED_ATTRIBS 
    };
    printf("Linking Shader\n");
    TransformShader mcBufferShader("./shaders/mc.vs", "./shaders/mc.gs", params);
    printf("Done Shader creation\n");
    // generate a cube buffer;
    std::vector<float> cubeVertices;
    for (int i = 0; i < GLOBAL_CHUNK_CONFIG.numVoxelsPerDim; i++)
    {
        float x = static_cast<float>(i) * GLOBAL_CHUNK_CONFIG.voxelSize;
        for (int j = 0; j < GLOBAL_CHUNK_CONFIG.numVoxelsPerDim; j++)
        {
            float y = static_cast<float>(j) * GLOBAL_CHUNK_CONFIG.voxelSize;
            for (int k = 0; k < GLOBAL_CHUNK_CONFIG.numVoxelsPerDim; k++)
            {
                float z = static_cast<float>(k) * GLOBAL_CHUNK_CONFIG.voxelSize;
                cubeVertices.push_back(x);
                cubeVertices.push_back(y);
                cubeVertices.push_back(z);
            }
        }
    }

    GLuint transformFeedbackObj;
    glGenTransformFeedbacks(1, &transformFeedbackObj);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transformFeedbackObj);

    mcBufferShader.use();
    mcBufferShader.setVec3("uChunkPosition", glm::value_ptr(chunkPosition));
    mcBufferShader.setFloat("uNumVoxelsPerDim", static_cast<float>(GLOBAL_CHUNK_CONFIG.numVoxelsPerDim));
    mcBufferShader.setFloat("uVoxelSize", GLOBAL_CHUNK_CONFIG.voxelSize);

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
    GLuint mcOutputBuffer;
    glGenBuffers(1, &mcOutputBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mcOutputBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * cubeVertices.size() * 9 , nullptr, GL_STATIC_DRAW);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mcOutputBuffer); 

    glViewport(0, 0, WIDTH, HEIGHT);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_RASTERIZER_DISCARD);
    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_3D, renderImageFramebuffer.texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, noiseTextures.noise1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, noiseTextures.noise2);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, noiseTextures.noise3);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_3D, noiseTextures.noise4);

    glBeginTransformFeedback(GL_TRIANGLES);
    glDrawArrays(GL_POINTS, 0, cubeVertices.size());
    glEndTransformFeedback();

    
    //std::cout << "===========================" << std::endl;
    //print_vec(chunkPosition);
    //float subdata[32*32*32][3];
    //glGetBufferSubData(GL_ARRAY_BUFFER, 0, 10000, subdata);
    //for (int i = 0; i < 30; i += 2)
    //{
        //print_vec(glm::vec3(subdata[i][0], subdata[i][1], subdata[i][2]));
    //}

    glBindVertexArray(0);
    glDisable(GL_RASTERIZER_DISCARD);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
    glFlush();
    glUseProgram(0);

    return { cubeVao, transformFeedbackObj, mcOutputBuffer } ;
}

void createChunk(GLuint quadVao, Texture3DBuffer textureBuffer, Shader textureProgram, NoiseTextures noiseTextures)
{
    if (chunksToCreate.empty())
    {
        return;
    }

    printf("Creating a chunk\n");
    glm::vec3 chunkToCreate = chunksToCreate.back();
    chunksToCreate.pop_back();
    ChunkMeshOutput meshOutput = generateChunkMesh(chunkToCreate, textureBuffer, noiseTextures);
    Chunk chunk { .meshOutput = meshOutput, .position = chunkToCreate };
    createdChunks.push_back(chunk);
    printf("Finished creating a chunk\n");
}

void debugCallback(GLenum source, GLenum type, GLuint id, GLenum sev, GLsizei length, const GLchar* message, const void* userParam)
{
    std::cout << source << ":" << type << ":" << id << ":" << sev << message << std::endl;
}

int main()
{
    srand(10);
    const glm::vec3 lightDir(1.0, 1.0, 1.0);

    GLFWwindow* window = setupWindow(WIDTH, HEIGHT);
    setupGlad();
    glDebugMessageCallback(debugCallback, nullptr);

    glm::mat4 perspectiveTransform = glm::perspective(glm::radians(45.0), (double) WIDTH / HEIGHT, 0.5, 10000.0);

    Shader finalRenderShader("./shaders/basic.vs", "./shaders/basic.fs");
    finalRenderShader.use();
    finalRenderShader.setInt("rockTexture", 0);
    finalRenderShader.setInt("groundTexture", 1);


    Texture3DBuffer renderImageFramebuffer = createFramebuffer();
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

        
    int layerIndex = 16;
    long frameCount = 0;

    GLuint noiseTexture1 = createNoiseTexture(noise1);
    GLuint noiseTexture2 = createNoiseTexture(noise2);
    GLuint noiseTexture3 = createNoiseTexture(noise3);
    GLuint noiseTexture4 = createNoiseTexture(noise4);

    NoiseTextures noiseTextures { noiseTexture1, noiseTexture2, noiseTexture3, noiseTexture4};
    initChunksToCreate();

    std::cout << "Creating chunks" << std::endl;

    GLuint groundTexture = generate_texture_2d("assets/leaves.jpg", GL_RGB);
    GLuint rockTexture = generate_texture_2d("assets/rock.jpg", GL_RGB);
    while (!glfwWindowShouldClose(window))
    {
        float currentTime = glfwGetTime();
        deltaTime = currentTime - previousTime;
        previousTime = currentTime;

        handleInputs(window);
        createChunk(quadVao,renderImageFramebuffer,text3DProgram, noiseTextures);

        glViewport(0, 0, WIDTH, HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, createdChunks[1].meshOutput.feedbackObj);
        finalRenderShader.use();
        glm::mat4 cameraTransfrom = camera->GetViewMatrix();
        finalRenderShader.setFloatMat4("uWorldTransform", (float*) glm::value_ptr(glm::mat4(1.0)));
        finalRenderShader.setFloatMat4("uCameraTransform", glm::value_ptr(cameraTransfrom));
        finalRenderShader.setFloatMat4("uPerspectiveTransform", glm::value_ptr(perspectiveTransform));
        finalRenderShader.setVec3("uLightDir", (float*) glm::value_ptr(lightDir));
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rockTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, groundTexture);

        glEnable(GL_DEPTH_TEST);
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for (Chunk chunk : createdChunks)
        {
            
            glBindVertexArray(chunk.meshOutput.vao); 
            glBindBuffer(GL_ARRAY_BUFFER, chunk.meshOutput.bufferBase);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) 0);

            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) (3 * sizeof(float)));
            glDrawTransformFeedback(GL_TRIANGLES, chunk.meshOutput.feedbackObj);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
        frameCount += 1; 
    }

    return 0;
}

void glfwErrorCallback(int code, const char* desc)
{
  printf("GLFW Error: %d :: %s\n", code, desc);
}

GLFWwindow* setupWindow(int width, int height)
{
    glfwInit();
    glfwSetErrorCallback(glfwErrorCallback);

    //glfwWindowHint(GLFW_OPENGL_CORE_PROFILE, GL_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

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
