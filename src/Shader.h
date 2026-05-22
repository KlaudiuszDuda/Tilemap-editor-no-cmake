#pragma once

#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "types.h"

#define PRODUCTION_BUILD 0;

class Shader
{
public:
    inline void LoadShader(const char* vShaderFile, const char* fShaderFile) {

        // 1. retrieve the vertex/fragment source code from filePath
        std::string vertexCode;
        std::string fragmentCode;
#ifdef PRODUCTION_BUILD == 0
        try
        {
            // open files
            std::ifstream vertexShaderFile(vShaderFile);
            if (!vertexShaderFile.is_open()) {
                std::cerr << "Failed to open vertex shader: " << vShaderFile << std::endl;
            }
            std::ifstream fragmentShaderFile(fShaderFile);
            if (!fragmentShaderFile.is_open()) {
                std::cerr << "Failed to open vertex shader: " << vShaderFile << std::endl;
            }
            std::stringstream vShaderStream, fShaderStream;
            // read file's buffer contents i32o streams
            vShaderStream << vertexShaderFile.rdbuf();
            fShaderStream << fragmentShaderFile.rdbuf();
            // close file handlers
            vertexShaderFile.close();
            fragmentShaderFile.close();
            // convert stream i32o string
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();

        }
        catch (std::exception e)
        {
            std::cerr << "ERROR::SHADER: Failed to read shader files" << std::endl;
        }
#else
        // open files
        std::ifstream vertexShaderFile(vShaderFile);
        std::ifstream fragmentShaderFile(fShaderFile);
        std::stringstream vShaderStream, fShaderStream;
        // read file's buffer contents i32o streams
        vShaderStream << vertexShaderFile.rdbuf();
        fShaderStream << fragmentShaderFile.rdbuf();
        // close file handlers
        vertexShaderFile.close();
        fragmentShaderFile.close();
        // convert stream i32o string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
#endif
        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();

        // compile shader
        u32 sVertex, sFragment;
        // vertex Shader
        sVertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(sVertex, 1, &vShaderCode, NULL);
        glCompileShader(sVertex);
#ifdef PRODUCTION_BUILD == 0
        checkCompileErrors(sVertex, "VERTEX");
#endif
        // fragment Shader
        sFragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(sFragment, 1, &fShaderCode, NULL);
        glCompileShader(sFragment);
#ifdef PRODUCTION_BUILD == 0
        checkCompileErrors(sFragment, "FRAGMENT");
#endif
        shaderid = glCreateProgram();
        glAttachShader(shaderid, sVertex);
        glAttachShader(shaderid, sFragment);

        glLinkProgram(shaderid);
#ifdef PRODUCTION_BUILD == 0
        checkCompileErrors(shaderid, "PROGRAM");
#endif
        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(sVertex);
        glDeleteShader(sFragment);
    }
    inline u32& getID() { return shaderid; }
    inline void bind() { glUseProgram(shaderid); }
    // Shadername, uniformname, value_of_uniform.
    inline void SetFloat(const char* uniformname, f32 value) {
        glUniform1f(glGetUniformLocation(shaderid, uniformname), value);
    }
    inline void SetInt(const char* uniformname, i32 value) {
        glUniform1i(glGetUniformLocation(shaderid, uniformname), value);
    }
    inline void SetUint(const char* uniformname, u32 value) {
        glUniform1ui(glGetUniformLocation(shaderid, uniformname), value);
    }
    inline void SetVector2i(const char* uniformname, i32 x, i32 y) {
        i32 test = glGetUniformLocation(shaderid, uniformname);
        glUniform2i(glGetUniformLocation(shaderid, uniformname), x, y);
    }
    inline void SetVector2i(const char* uniformname, glm::ivec2 value) {
        glUniform2i(glGetUniformLocation(shaderid, uniformname), value.x, value.y);
    }
    inline void SetVector3i(const char* uniformname, i32 x, i32 y, i32 z) {
        glUniform3i(glGetUniformLocation(shaderid, uniformname), x, y, z);
    }
    inline void SetVector3i(const char* uniformname, glm::ivec3 value) {
        glUniform3i(glGetUniformLocation(shaderid, uniformname), value.x, value.y, value.z);
    }
    inline void SetVector4i(const char* uniformname, i32 x, i32 y, i32 z, i32 w) {
        glUniform4i(glGetUniformLocation(shaderid, uniformname), x, y, z, w);
    }
    inline void SetVector4i(const char* uniformname, glm::ivec4 value) {
        glUniform4i(glGetUniformLocation(shaderid, uniformname), value.x, value.y, value.z, value.w);
    }
    inline void SetVector2ui(const char* uniformname, u32 x, u32 y) {
        glUniform2ui(glGetUniformLocation(shaderid, uniformname), x, y);
    }
    inline void SetVector2ui(const char* uniformname, glm::uvec2 value) {
        glUniform2ui(glGetUniformLocation(shaderid, uniformname), value.x, value.y);
    }
    inline void SetVector3ui(const char* uniformname, u32 x, u32 y, u32 z) {
        glUniform3ui(glGetUniformLocation(shaderid, uniformname), x, y, z);
    }
    inline void SetVector3ui(const char* uniformname, glm::uvec3 value) {
        glUniform3ui(glGetUniformLocation(shaderid, uniformname), value.x, value.y, value.z);
    }
    inline void SetVector4ui(const char* uniformname, u32 x, u32 y, u32 z, u32 w) {
        glUniform4ui(glGetUniformLocation(shaderid, uniformname), x, y, z, w);
    }
    inline void SetVector4ui(const char* uniformname, glm::uvec4 value) {
        glUniform4ui(glGetUniformLocation(shaderid, uniformname), value.x, value.y, value.z, value.w);
    }
    inline void SetVector2f(const char* uniformname, f32 x, f32 y) {
        glUniform2f(glGetUniformLocation(shaderid, uniformname), x, y);
    }
    inline void SetVector2f(const char* uniformname, glm::vec2 value) {
        glUniform2f(glGetUniformLocation(shaderid, uniformname), value.x, value.y);
    }
    inline void SetVector3f(const char* uniformname, f32 x, f32 y, f32 z) {
        glUniform3f(glGetUniformLocation(shaderid, uniformname), x, y, z);
    }
    inline void SetVector3f(const char* uniformname, glm::vec3 value) {
        glUniform3f(glGetUniformLocation(shaderid, uniformname), value.x, value.y, value.z);
    }
    inline void SetVector4f(const char* uniformname, f32 x, f32 y, f32 z, f32 w) {
        glUniform4f(glGetUniformLocation(shaderid, uniformname), x, y, z, w);
    }
    inline void SetVector4f(const char* uniformname, glm::vec4 value) {
        glUniform4f(glGetUniformLocation(shaderid, uniformname), value.x, value.y, value.z, value.w);
    }
    inline void SetVector2d(const char* uniformname, f64 x, f64 y) {
        glUniform2d(glGetUniformLocation(shaderid, uniformname), x, y);
    }
    inline void SetVector2d(const char* uniformname, glm::dvec2 value) {
        glUniform2d(glGetUniformLocation(shaderid, uniformname), value.x, value.y);
    }
    inline void SetVector3d(const char* uniformname, f64 x, f64 y, f64 z) {
        glUniform3d(glGetUniformLocation(shaderid, uniformname), x, y, z);
    }
    inline void SetVector3d(const char* uniformname, glm::dvec3 value) {
        glUniform3d(glGetUniformLocation(shaderid, uniformname), value.x, value.y, value.z);
    }
    inline void SetVector4d(const char* uniformname, f64 x, f64 y, f64 z, f64 w) {
        glUniform4d(glGetUniformLocation(shaderid, uniformname), x, y, z, w);
    }
    inline void SetVector4d(const char* uniformname, glm::dvec4 value) {
        glUniform4d(glGetUniformLocation(shaderid, uniformname), value.x, value.y, value.z, value.w);
    }
    inline void SetMatrix4(const char* uniformname, const glm::mat4& matrix) {
        glUniformMatrix4fv(glGetUniformLocation(shaderid, uniformname), 1, false, glm::value_ptr(matrix));
    }

    inline void SetFloat(int uniformid, f32 value) {
        glUniform1f(uniformid, value);
    }
    inline void SetInt(int uniformid, i32 value) {
        glUniform1i(uniformid, value);
    }
    inline void SetUint(int uniformid, u32 value) {
        glUniform1ui(uniformid, value);
    }
    inline void SetVector2i(int uniformid, i32 x, i32 y) {
        glUniform2i(uniformid, x, y);
    }
    //inline void SetVector2i(int uniformid, glm::ivec2 value) {
    //    glUniform2i(uniformid, value.x, value.y);
    //}
    inline void SetVector3i(int uniformid, i32 x, i32 y, i32 z) {
        glUniform3i(uniformid, x, y, z);
    }
    //inline void SetVector3i(int uniformid, glm::ivec3 value) {
    //    glUniform3i(uniformid, value.x, value.y, value.z);
    //}
    inline void SetVector4i(int uniformid, i32 x, i32 y, i32 z, i32 w) {
        glUniform4i(uniformid, x, y, z, w);
    }
    //inline void SetVector4i(int uniformid, glm::ivec4 value) {
    //    glUniform4i(uniformid, value.x, value.y, value.z, value.w);
    //}
    inline void SetVector2ui(int uniformid, u32 x, u32 y) {
        glUniform2ui(uniformid, x, y);
    }
    //inline void SetVector2ui(int uniformid, glm::uvec2 value) {
    //    glUniform2ui(uniformid, value.x, value.y);
    //}
    inline void SetVector3ui(int uniformid, u32 x, u32 y, u32 z) {
        glUniform3ui(uniformid, x, y, z);
    }
    //inline void SetVector3ui(int uniformid, glm::uvec3 value) {
    //    glUniform3ui(uniformid, value.x, value.y, value.z);
    //}
    inline void SetVector4ui(int uniformid, u32 x, u32 y, u32 z, u32 w) {
        glUniform4ui(uniformid, x, y, z, w);
    }
    //inline void SetVector4ui(int uniformid, glm::uvec4 value) {
    //    glUniform4ui(uniformid, value.x, value.y, value.z, value.w);
    //}
    inline void SetVector2f(int uniformid, f32 x, f32 y) {
        glUniform2f(uniformid, x, y);
    }
    //inline void SetVector2f(int uniformid, glm::vec2 value) {
    //    glUniform2f(uniformid, value.x, value.y);
    //}
    inline void SetVector3f(int uniformid, f32 x, f32 y, f32 z) {
        glUniform3f(uniformid, x, y, z);
    }
    //inline void SetVector3f(int uniformid, glm::vec3 value) {
    //    glUniform3f(uniformid, value.x, value.y, value.z);
    //}
    inline void SetVector4f(int uniformid, f32 x, f32 y, f32 z, f32 w) {
        glUniform4f(uniformid, x, y, z, w);
    }
    //inline void SetVector4f(int uniformid, glm::vec4 value) {
    //    glUniform4f(uniformid, value.x, value.y, value.z, value.w);
    //}
    inline void SetVector2d(int uniformid, f64 x, f64 y) {
        glUniform2d(uniformid, x, y);
    }
    //inline void SetVector2d(int uniformid, glm::dvec2 value) {
    //    glUniform2d(uniformid, value.x, value.y);
    //}
    inline void SetVector3d(int uniformid, f64 x, f64 y, f64 z) {
        glUniform3d(uniformid, x, y, z);
    }
    //inline void SetVector3d(int uniformid, glm::dvec3 value) {
    //    glUniform3d(uniformid, value.x, value.y, value.z);
    //}
    inline void SetVector4d(int uniformid, f64 x, f64 y, f64 z, f64 w) {
        glUniform4d(uniformid, x, y, z, w);
    }
    //inline void SetVector4d(int uniformid, glm::dvec4 value) {
    //    glUniform4d(uniformid, value.x, value.y, value.z, value.w);
    //}
    //inline void SetMatrix4(int uniformid, const glm::mat4& matrix) {
    //    glUniformMatrix4fv(uniformid, 1, false, glm::value_ptr(matrix));
    //}

private:
#ifdef PRODUCTION_BUILD == 0
    inline void checkCompileErrors(const u32& object, const char* type)
    {
        i32 success;
        char infoLog[1024];
        if (strcmp(type, "PROGRAM") != 0)
        {
            glGetShaderiv(object, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(object, 1024, NULL, infoLog);
                std::cerr << "| ERROR::SHADER: Compile-time error: Type: " << type << "\n"
                    << infoLog << "\n -- --------------------------------------------------- -- "
                    << std::endl;
            }
        }
        else
        {
            glGetProgramiv(object, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(object, 1024, NULL, infoLog);
                std::cerr << "| ERROR::Shader: Link-time error: Type: " << type << "\n"
                    << infoLog << "\n -- --------------------------------------------------- -- "
                    << std::endl;
            }
        }
    }
#endif
    u32 shaderid;
};