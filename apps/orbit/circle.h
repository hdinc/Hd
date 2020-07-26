#include <Hd.h>
#include <glm/glm.hpp>

class circle {
public:
    static void init_gldata(Hd::Shader* s, glm::mat4* v, glm::mat4* p);
    static void destroy_gldata();

    void draw();
    void set_radius(float r);
    void set_location(glm::vec2 loc);
    circle(float radius, glm::vec2 location);
    ~circle();

private:
    static int vcount;
    static GLuint vao, vbo;
    static Hd::Shader* shader;
    static glm::mat4* viewport;
    static glm::mat4* projection;
    static GLint MVP, COLOR;

    float radius;
    glm::vec3 color;
    glm::vec2 location;
    glm::mat4 mvp;
    glm::mat4 model;
};
