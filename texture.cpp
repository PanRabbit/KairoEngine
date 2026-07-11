#include "texture.h"
#include <iostream>
#include <glad/glad.h>
#include "stb_image.h"

Texture::Texture(const std::string& path)
{
    stbi_set_flip_vertically_on_load(true); // fixes image being upsidedown
    unsigned char *image_data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);   
    
    GLenum format;
    if (nrChannels == 3) 
    {
        format = GL_RGB;
    }
    else if (nrChannels == 4)
    {
        format = GL_RGBA;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image_data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(image_data);
}

void Texture::bind()
{
    glBindTexture(GL_TEXTURE_2D, ID);
}

Texture::~Texture() 
{
    glDeleteTextures(1, &ID);
}
