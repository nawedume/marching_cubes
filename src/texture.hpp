#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

GLuint generate_texture_2d(std::string texture_path, GLenum format)
{
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;
    unsigned char* textureData = stbi_load(texture_path.c_str(), &width, &height, &nrChannels, 0);
    if (!textureData)
    { 
        std::cerr << "Failed to load image data for container.jpeg" << std::endl;
        exit(1);
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, textureData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(textureData);

    return texture;
}

