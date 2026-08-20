#pragma once
#include <string>
#include <vector>

class Texture
{
public:
    // code that others can reference and call
    unsigned int ID;

    Texture(const std::string& path);

    ~Texture();

    void bind(unsigned int slot = 0);


private:
    // code that is only accessible within this class
    int width, height, nrChannels;
};


class CubeMapTexture
{
public:
    unsigned int ID;

    CubeMapTexture(std::vector<std::string> faces);

    ~CubeMapTexture();
};