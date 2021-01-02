#pragma once

#include <vector>
#include <Hd/Shader.h>
#include <glm/glm.hpp>

class balls {
private:
    static const int mCircleVertexCount;
    int mCount;
    float mRadius;
    glm::vec2 mBorder;

private:
    GLuint mVao, mVbo, mLbo;
    glm::mat4& mProjection;
    Hd::Shader mShader;

    glm::vec2* mSpeed;
    glm::vec2* mLoc;

    bool mChanged = true;

    void initGL();
    void destroyGL();
    void initBalls();
    void borderBounce();
    void ballBounce();

public:
    balls(glm::mat4& projection, int count, float radius, glm::vec2 border);
    ~balls();

    void setRadius(float r);
    void step(float dt);
    void draw();
    float calculateTotalEnergy();
};
