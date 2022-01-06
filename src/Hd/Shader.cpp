#include <stdio.h>
#include <stdlib.h>
#include <Hd/Shader.h>

#ifdef _WIN32
    #include <malloc.h>
    #define alloca _alloca
#endif

static const char* GetFile(const char* filename);

namespace Hd {

Shader::Shader(const char* vertex, const char* fragment)
{

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

    const char* vss = GetFile(vertex);
    glShaderSource(vs, 1, &vss, NULL);
    free((void*)vss);
    glCompileShader(vs);
    CheckError(vs, GL_COMPILE_STATUS);

    const char* fss = GetFile(fragment);
    glShaderSource(fs, 1, &fss, NULL);
    free((void*)fss);
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

    const char* vss = GetFile(vertex);
    glShaderSource(vs, 1, &vss, NULL);
    free((void*)vss);
    glCompileShader(vs);
    CheckError(vs, GL_COMPILE_STATUS);

    const char* fss = GetFile(fragment);
    glShaderSource(fs, 1, &fss, NULL);
    free((void*)fss);
    glCompileShader(fs);
    CheckError(fs, GL_COMPILE_STATUS);

    const char* gss = GetFile(geometry);
    glShaderSource(gs, 1, &gss, NULL);
    free((void*)gss);
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

    const char* css = GetFile(compute);
    glShaderSource(cs, 1, &css, NULL);
    free((void*)css);
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

static const char* GetFile(const char* filename)
{
    char* buffer = 0;
    long length;
    FILE* f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "cannot open %s\n", filename);
        exit(1);
    }
    fseek(f, 0, SEEK_END);
    length = ftell(f);
    fseek(f, 0, SEEK_SET);
    buffer = (char*)malloc(length + 1);
    fread(buffer, 1, length, f);
    fclose(f);
    buffer[length] = '\0';
    return buffer;
}
