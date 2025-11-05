#include "Shader.h"
#include <iostream> 
#include <fstream> 
#include <sstream> 
#include <iostream> 

Shader::Shader(const std::string& vetexPath, const std::string& fragmentPath) {
    std::string vertexSource = loadShaderSource(vetexPath); 
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
    glUniform4f(getUniformLocation(name), v0, v1, v2, v3);
}

void Shader::setUniformVec4(const std::string& name, const glm::vec4& vec){
    glUniform4f(getUniformLocation(name), vec.x, vec.y, vec.z, vec.w); 
}

void Shader::setUniformMat4(const std::string& name, const glm::mat4& matrix){
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &matrix[0][0]); 
}

GLint Shader::getUniformLocation(const std::string& name){
    // kiedys dodac mape (cache) do przechowywania lokalizacji 
    return glGetUniformLocation(m_RendererID, name.c_str()); 
}

// funkcje pomocnicze skopiowane z main.cpp
std::string Shader::loadShaderSource(const std::string& filepath){
    std::ifstream file(filepath); 
    if(!file.is_open()){
        std::cerr << "Nie można otworzyć pliku shadera: " << filepath << std::endl; 
        return ""; 
    }

    // zmieniona metoda odczytu danych linia po lini 
    std::string line; 
    std::stringstream ss; 

    while(std::getline(file, line)){
        // wczytywanie linie i dodawanie nowej 
        ss << line << "\n"; 
    }

    return ss.str(); 
}

GLuint Shader::compileShader(GLuint type, const std::string& source){
    GLuint id = glCreateShader(type); 
    const char* stc = source.c_str(); 
    glShaderSource(id, 1, &stc, nullptr);
    glCompileShader(id); 

    int result; 
    glGetShaderiv(id, GL_COMPILE_STATUS, &result); 
    if (result == GL_FALSE){
        int length; 
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cerr << "Błąd kompilacji shadera: " << (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment") << "): " 
                    << message << std::endl; 
        glDeleteShader(id);
        return 0;
    }
    return id; 
}

GLuint Shader::createShaderProgram(const std::string& vertexShader, const std::string& fragmentShader){
    GLuint program = glCreateProgram();
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShader); 
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader); 

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs); 
    glDeleteShader(fs); 

    return program; 
}