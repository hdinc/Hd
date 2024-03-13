#pragma once

#include <glad/glad.h>

namespace Hd {

class Shader {

public:
    Shader(const char* vertex, const char* fragment);
    Shader(const char* vertex, const char* geometry, const char* fragment);
    Shader(const char* compute);

    Shader(Shader&& other);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    void Bind();
    GLuint Id();

private:
    void CheckError(GLuint id, GLenum pname);
    GLuint m_ProgramId;
};

inline void Shader::Bind() { glUseProgram(m_ProgramId); }
inline GLuint Shader::Id() { return m_ProgramId; }

inline Shader::Shader(Shader&& other)
{
    m_ProgramId = other.m_ProgramId;
    other.m_ProgramId = 0;
}

}
