#pragma once 
#include <string> 
#include <GL/glew.h> 
#include <glm/glm.hpp>


class Shader {
    public: 
        Shader(const std::string& vertexPath, const std::string& fragmentPath); 
        ~Shader(); 

        void bind() const; 
        void unbind() const; 

        // funkcje do ustawiania uniformow 
        void setUniform4f(const std::string& name, float v0, float v1, float v2, float v3); 
        void setUniformVec4(const std::string& name, const glm::vec4& vec); 
        void setUniformMat4(const std::string& name, const glm::mat4& matrix); 
    
    private: 
        GLuint m_RendererID;

        std::string loadShaderSource(const std::string& filePath);
        GLuint compileShader(GLuint type, const std::string& source); 
        GLuint createShaderProgram(const std::string& vertexShader, const std::string& fragmentShader); 
        GLint getUniformLocation(const std::string& name);
    
};