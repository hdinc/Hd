#include <stdio.h>
#include <stdlib.h>
#include <Hd/Shader.h>
#include <string>

#ifdef _WIN32
    #include <malloc.h>
    #define alloca _alloca
#endif


namespace Hd {

#ifdef __EMSCRIPTEN__
// glsl 330 as glsl es 300 for webgl 2. emscripten handles the explicit
// uniform locations (-sGL_EXPLICIT_UNIFORM_LOCATION), so the extension goes.
static void shader_source(GLuint shader, const char* source)
{
    std::string s = source;
    auto replace = [&](const std::string& from, const std::string& to) {
        if (auto i = s.find(from); i != std::string::npos) s.replace(i, from.size(), to);
    };
    replace("#version 330 core", "#version 300 es\nprecision highp float;\nprecision highp int;\nprecision highp sampler2D;");
    replace("#extension GL_ARB_explicit_uniform_location : require", "");
    const char* p = s.c_str();
    glShaderSource(shader, 1, &p, NULL);
}
#else
static void shader_source(GLuint shader, const char* source)
{
    glShaderSource(shader, 1, &source, NULL);
}
#endif

Shader::Shader(const char* vertex, const char* fragment)
{

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

    shader_source(vs, vertex);
    glCompileShader(vs);
    CheckError(vs, GL_COMPILE_STATUS);

    shader_source(fs, fragment);
    glCompileShader(fs);
    CheckError(fs, GL_COMPILE_STATUS);

    m_ProgramId = glCreateProgram();
    glAttachShader(m_ProgramId, vs);
    glAttachShader(m_ProgramId, fs);
    glLinkProgram(m_ProgramId);
    CheckError(m_ProgramId, GL_LINK_STATUS);
    glValidateProgram(m_ProgramId);
    CheckError(m_ProgramId, GL_VALIDATE_STATUS);

    glDeleteShader(vs);
    glDeleteShader(fs);
}

Shader::Shader(const char* vertex, const char* geometry, const char* fragment)
{
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint gs = glCreateShader(GL_GEOMETRY_SHADER);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

    shader_source(vs, vertex);
    glCompileShader(vs);
    CheckError(vs, GL_COMPILE_STATUS);

    shader_source(fs, fragment);
    glCompileShader(fs);
    CheckError(fs, GL_COMPILE_STATUS);

    shader_source(gs, geometry);
    glCompileShader(gs);
    CheckError(gs, GL_COMPILE_STATUS);

    m_ProgramId = glCreateProgram();
    glAttachShader(m_ProgramId, vs);
    glAttachShader(m_ProgramId, fs);
    glAttachShader(m_ProgramId, gs);
    glLinkProgram(m_ProgramId);
    CheckError(m_ProgramId, GL_LINK_STATUS);
    glValidateProgram(m_ProgramId);
    CheckError(m_ProgramId, GL_VALIDATE_STATUS);

    glDeleteShader(vs);
    glDeleteShader(fs);
    glDeleteShader(gs);
}

Shader::Shader(const char* compute)
{
    GLuint cs = glCreateShader(GL_COMPUTE_SHADER);

    shader_source(cs, compute);
    glCompileShader(cs);
    CheckError(cs, GL_COMPILE_STATUS);

    m_ProgramId = glCreateProgram();
    glAttachShader(m_ProgramId, cs);
    glLinkProgram(m_ProgramId);
    CheckError(m_ProgramId, GL_LINK_STATUS);
    glValidateProgram(m_ProgramId);
    CheckError(m_ProgramId, GL_VALIDATE_STATUS);

    glDeleteShader(cs);
}

Shader::~Shader() { glDeleteProgram(m_ProgramId); }

void Shader::CheckError(GLuint id, GLenum pname)
{
    int result;
    if (pname == GL_COMPILE_STATUS) {
        glGetShaderiv(id, pname, &result);
        if (result == GL_FALSE) {
            int length;
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
            char* message = (char*)alloca(length);
            glGetShaderInfoLog(id, length, &length, message);
            int type;
            glGetShaderiv(id, GL_SHADER_TYPE, &type);
            printf("Failed to complie %s shader ; \n",
                (type == GL_VERTEX_SHADER ? "vertex" : "fragment"));
            printf("%s\n", message);
            glDeleteShader(id);
            exit(-1);
        }
    } else if (pname == GL_LINK_STATUS || pname == GL_VALIDATE_STATUS) {
        glGetProgramiv(id, pname, &result);
        if (result == GL_FALSE) {
            int length;
            glGetProgramiv(id, GL_INFO_LOG_LENGTH, &length);
            char* message = (char*)alloca(length);
            glGetProgramInfoLog(id, length, &length, message);
            printf("Failed to %s program;\n%s\n",
                pname == GL_LINK_STATUS ? "link" : "validate", message);
            glDeleteProgram(id);
            exit(-1);
        }
    }
}

}
