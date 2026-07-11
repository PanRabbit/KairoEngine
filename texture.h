#pragma once
#include <string>

class Texture
{
public:
    // code that others can reference and call
    unsigned int ID;

    Texture(const std::string& path);

    ~Texture();

    void bind();

private:
    // code that is only accessible within this class
    int width, height, nrChannels;
};