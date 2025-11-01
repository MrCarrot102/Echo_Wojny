#include "Shader.h"
#include <iostream> 
#include <fstream> 
#include <sstream> 

Shader::Shader(const std::string& vetexPath, cosnt std::string& framgentPath) {
    std::string vertexSource = loadShaderSource(vertexPath); 
    std::string fragmentSource = loadShaderSource(fragmentPath); 
    m_RendererID = createShaderProgram(vertexSource, fragmentSource); 
}

Shader::~Shader() {
    glDeleteProgram(m_RendererID); 
}

void Shader::bind() const {
    glUseProgram(m_RendererID);
}

void Shader::unbind() const {
    glUseProgram(0);
}

// implementacja setuniform
void Shader::setUniform4f(const std::string& name, float v0, float v1, float v2, float v3) {
    glUnifomr4f(getUniformLocation(name), v0, v1, v2, v3);
}

void Shader::setUniformVec4(const setd::string& name, const glm::vec4& vec){
    flUnitform4f(getUniformLocation(name), vec.x, vec.y, vec.z, vec.w); 
}

