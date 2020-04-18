#pragma once

#include <glad/glad.h>

namespace Hd {

class Shader {

public:
    GLuint m_ProgramId;

    Shader(const char* vertex, const char* fragment);
    Shader(const char* vertex, const char* geometry, const char* fragment);
    Shader(const char* compute);
    ~Shader();
    void Use();

private:
    void CheckError(GLuint id, GLenum pname);
};

inline void Shader::Use()
{
    glUseProgram(m_ProgramId);
}

}
