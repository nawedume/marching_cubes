#ifndef TSHADER_H
#define TSHADER_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdlib.h>

struct TransformOutputParams
{
    const char** varyings;
    const GLsizei varyingsCount;
    GLenum bufferMode;
};

class TransformShader
{

public:
    unsigned int ID; // program ID
    
    TransformShader(const char* vertexPath, const char* geometryShaderPath, TransformOutputParams params)
    {
        std::string vertexCode;
        std::string geometryCode;
        std::ifstream vShaderFile;
        std::ifstream vGeoFile;

        vShaderFile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
        vGeoFile.exceptions( std::ifstream::failbit | std::ifstream::badbit );
        
        printf("Opening files\n");
        try
        {
            // open files
            vShaderFile.open(vertexPath);
            vGeoFile.open(geometryShaderPath);
            std::stringstream vShaderStream, fShaderStream;

            // read file buffer
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << vGeoFile.rdbuf();
            vShaderFile.close();
            vGeoFile.close();

            vertexCode = vShaderStream.str();
            geometryCode = fShaderStream.str();
        }
        catch(std::ifstream::failure e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
            exit(1);
        }
        
        const char* vShaderCode = vertexCode.c_str();
        const char* gShaderCode = geometryCode.c_str();
        printf("Got shader code %d, %d\n", vertexCode.size(), geometryCode.size());

        unsigned int vertex, geometry;
        int success;
        char infoLog[512];

        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertex, 512, NULL, infoLog);
            std::cout << "avasaERROR::VERTEX_SHADER::COMPILATION_FAILED::" << vertexPath << ":" << infoLog << std::endl;
            exit(1);
        }
        printf("Created vertex shader\n");

        geometry = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometry, 1, &gShaderCode, NULL);
        glCompileShader(geometry);
        glGetShaderiv(geometry, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(geometry, 512, NULL, infoLog);
            std::cout << "ERROR::GEOMETRY_SHADER::COMPILATION_FAILED" << infoLog << std::endl;
            exit(1);
        }
        printf("Created geo shader\n");

        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, geometry);
        glTransformFeedbackVaryings(ID, params.varyingsCount, params.varyings, params.bufferMode);
        printf("Linking....\n");
        glLinkProgram(ID);
        printf("Linked\n");
        glGetProgramiv(ID, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(ID, 512, NULL, infoLog);
            std::cout << "ERROR::PROGRAM::LINK_FAILED" << infoLog << std::endl;
            exit(1);
        }
        printf("Created and linked program\n");
        glDeleteShader(vertex);
        glDeleteShader(geometry);
    }

    void use() const
    {
        glUseProgram(ID);
    }

    // utility unifrom functions
    void setBool(const std::string &name, bool value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int) value);
    }
    void setInt(const std::string &name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setFloat(const std::string &name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setFloatMat4(const std::string& name, float* value)
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, value);
    }
    void setVec3(const std::string& name, float* value) const
    {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, value);
    }

    void setFloatArray(const std::string& name, int size, float* value)
    {
      glUniform1fv(glGetUniformLocation(ID, name.c_str()), size, value);
    }

    void setFloatArray(const std::string& name, int size, const int* value)
    {
      glUniform1iv(glGetUniformLocation(ID, name.c_str()), size, value);
    }
};

#endif
