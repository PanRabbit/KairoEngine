#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h> // Include glad to get all the required OpenGL headers
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader
{
public:
    // The program ID
    unsigned int ID;

    // variables to store the shader file paths
    std::string vertexSourcePath;
    std::string fragmentSourcePath;


    // Constructor reads and builds the shader
    Shader(const char* vertexPath, const char* fragmentPath)
    {
        this->vertexSourcePath = vertexPath;
        this->fragmentSourcePath = fragmentPath;
        compile();
    }

    // make the compilation of shaders re-callable
    void compile()
    {
                // 1. Retrieve the vertex/fragment source code from filePath
                std::string vertexCode;
                std::string fragmentCode;
                std::ifstream vShaderFile;
                std::ifstream fShaderFile;
        
                // Ensure ifstream objects can throw exceptions:
                vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
                fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
                
                try
                {
                    // Open files
                    vShaderFile.open(vertexSourcePath);
                    fShaderFile.open(fragmentSourcePath);
                    std::stringstream vShaderStream, fShaderStream;
                    
                    // Read file's buffer contents into streams
                    vShaderStream << vShaderFile.rdbuf();
                    fShaderStream << fShaderFile.rdbuf();
                    
                    // Close file handlers
                    vShaderFile.close();
                    fShaderFile.close();
                    
                    // Convert stream into string
                    vertexCode   = vShaderStream.str();
                    fragmentCode = fShaderStream.str();
                }
                catch (std::ifstream::failure& e)
                {
                    std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
                }
                
                const char* vShaderCode = vertexCode.c_str();
                const char* fShaderCode = fragmentCode.c_str();
        
                // 2. Compile shaders
                unsigned int vertex, fragment;
                int success;
                char infoLog[512];
                
                // Vertex Shader
                vertex = glCreateShader(GL_VERTEX_SHADER);
                glShaderSource(vertex, 1, &vShaderCode, NULL);
                glCompileShader(vertex);
                
                // Print compile errors if any
                glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
                if(!success)
                {
                    glGetShaderInfoLog(vertex, 512, NULL, infoLog);
                    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
                };
                
                // Fragment Shader (The book skipped this bit using [...])
                fragment = glCreateShader(GL_FRAGMENT_SHADER);
                glShaderSource(fragment, 1, &fShaderCode, NULL);
                glCompileShader(fragment);
                
                // Print compile errors if any
                glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
                if(!success)
                {
                    glGetShaderInfoLog(fragment, 512, NULL, infoLog);
                    std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
                };
                
                // Shader Program
                ID = glCreateProgram();
                glAttachShader(ID, vertex);
                glAttachShader(ID, fragment);
                glLinkProgram(ID);
                
                // Print linking errors if any
                glGetProgramiv(ID, GL_LINK_STATUS, &success);
                if(!success)
                {
                    glGetProgramInfoLog(ID, 512, NULL, infoLog);
                    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
                }
                
                // Delete shaders; they’re linked into our program and no longer necessary
                glDeleteShader(vertex);
                glDeleteShader(fragment);
    }

    void reload()
    {
        // back up the current working shader ID
        unsigned int oldID = this->ID;

        // run the compilation
        compile();

        // only delete the old shader program is compilation is successful
        if (this->ID != oldID)
        {
            glDeleteProgram(oldID);
            std::cout << "Shader Hot-Reload Successful!" << std::endl;
        }
    }

    // Use/activate the shader
    void use() const
    {
        glUseProgram(ID);
    }

    // Utility uniform functions
    void setBool(const std::string &name, bool value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    
    void setInt(const std::string &name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    
    void setFloat(const std::string &name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setVec3(const std::string &name, glm::vec3 value) const
    {
        glUniform3f(glGetUniformLocation(ID, name.c_str()), value.x, value.y, value.z);
    }

    void setMat4(const std::string &name, glm::mat4 value) const
    {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
    }

    // Clean up shader
    ~Shader()
    {
        glDeleteProgram(ID);
    }
};

#endif