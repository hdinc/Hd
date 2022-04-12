#include <Hd.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>
#include <iostream>

#include "OBJ_Loader.h"

void guifunc();

int main(int argc, char** argv)
{
    (void)argc;

    Hd::Window& w = Hd::Window::getInstance();

    Hd::Resource& res = Hd::Resource::getInstance(argv[0]);

    Hd::Camera cam;
    cam.setLoc(glm::vec3(0, 5, 10));

    Hd::Gui g;
    g.Functions.add(guifunc);

    Hd::Shader shader(res.get_file("res/vertex.glsl"), res.get_file("res/texture.glsl"));
    shader.Bind();

    objl::Loader l;
    std::string filename = "../res/models/cube.obj";
    if (!l.LoadFile(filename)) {
        std::cerr << "cant load file: " << filename << std::endl;
    };

    auto mesh = l.LoadedMeshes[0];

    uint32_t vao, vbo, ibo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(objl::Vertex) * mesh.Vertices.size(), &mesh.Vertices[0], GL_STATIC_DRAW);
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * mesh.Indices.size(), &mesh.Indices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    Hd::Texture tex(res.get_file("res/diyojen.png"), res.get_file_size("res/diyojen.png"));
    tex.Bind(1);

    /* glPolygonMode(GL_BACK, GL_LINES); */
    glEnable(GL_DEPTH_TEST);

    while (!w.ShouldClose()) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        static glm::mat4 rot(1);
        rot = glm::rotate(glm::mat4(1), (float)glfwGetTime(), glm::vec3(0, 1, 0));

        w.PollEvents();
        glUniformMatrix4fv(0, 1, 0, glm::value_ptr(*cam.getVP() * rot));
        glDrawElements(GL_TRIANGLES, mesh.Indices.size(), GL_UNSIGNED_INT, 0);
        g.Draw();

        w.SwapBuffers();
    }
}

void guifunc()
{
    ImGui::Begin("3dtest");
    ImGui::Text("fps = %d", Hd::Window::getInstance().getFps());
    ImGui::End();
}
