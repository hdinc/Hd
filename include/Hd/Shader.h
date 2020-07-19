#pragma once

#include <glad/glad.h>

namespace Hd {

class Shader {

public:
    Shader(const char* vertex, const char* fragment);
    Shader(const char* vertex, const char* geometry, const char* fragment);
    Shader(const char* compute);
    ~Shader();

    void Bind();
    GLuint Id() { return m_ProgramId; }

private:
    void CheckError(GLuint id, GLenum pname);
    GLuint m_ProgramId;
};

inline void Shader::Bind() { glUseProgram(m_ProgramId); }

}
