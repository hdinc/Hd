#pragma once

#include <vector>
#include <Hd/Shader.h>
#include <glm/glm.hpp>

class balls {
private:
    static const int mCircleVertexCount;
    int mCount = 10;
    float mRadius;
    glm::vec2 mBorder;

private:
    GLuint mVao, mVbo, mLbo;
    glm::mat4& mProjection;
    Hd::Shader mShader;
    Hd::Shader mComputeShader;

    glm::vec2* mSpeed;
    glm::vec2* mLoc;

    void initGL();
    void destroyGL();
    void initBalls();

public:
    balls(glm::mat4& projection, float radius, glm::vec2 border);
    ~balls();

    void setRadius(float r);
    void step(float dt);
    void draw();
    float calculateTotalEnergy();
};
