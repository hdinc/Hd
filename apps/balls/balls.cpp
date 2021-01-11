#include "balls.h"
#include <cstring>
#include <glm/common.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>

typedef struct {
    glm::vec2 loc[2][10];
    glm::vec2 speed[2][10];
} lb;

static lb* lbp;
static int cindex = 1;

const int balls::mCircleVertexCount = 20;

balls::balls(glm::mat4& projection, float radius, glm::vec2 border)
    : mProjection(projection)
    , mShader("../res/shaders/balls.vert", "../res/shaders/balls.frag")
    , mComputeShader("../res/shaders/balls_compute.glsl")
    , mBorder(border)
{
    mRadius = radius;

    lbp = new lb();

    mLoc = lbp->loc[0];
    mSpeed = lbp->speed[0];

    initBalls();
    initGL();
}

balls::~balls() { delete lbp; }

void balls::draw()
{
    mShader.Bind();
    glUniform1i(3, cindex);
    glBindVertexArray(mVao);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, mCircleVertexCount, mCount);
}

void balls::initGL()
{
    setRadius(mRadius);

    mShader.Bind();
    glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(mProjection));

    glGenVertexArrays(1, &mVao);
    glBindVertexArray(mVao);
    glGenBuffers(1, &mVbo);
    glBindBuffer(GL_ARRAY_BUFFER, mVbo);

    float circle_vertices[mCircleVertexCount + 2][2];
    circle_vertices[0][0] = 1;
    circle_vertices[0][1] = 0;
    circle_vertices[mCircleVertexCount][0] = 1;
    circle_vertices[mCircleVertexCount][1] = 0;
    for (int i = 1; i < mCircleVertexCount; i++) {
        circle_vertices[i][0] = cos(i * 2 * M_PI / mCircleVertexCount);
        circle_vertices[i][1] = sin(i * 2 * M_PI / mCircleVertexCount);
    }

    glBufferData(GL_ARRAY_BUFFER, sizeof(circle_vertices), circle_vertices,
        GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindVertexArray(0);

    glGenBuffers(1, &mLbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mLbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(lb), lbp, GL_DYNAMIC_DRAW);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, mLbo);

    mComputeShader.Bind();
    glUniform1i(1, 1);
    glUniform2f(2, mBorder.x, mBorder.y);
    glUniform1f(3, mRadius);
}

void balls::destroyGL()
{
    glDeleteVertexArrays(1, &mVao);
    glDeleteBuffers(1, &mVbo);
    glDeleteBuffers(1, &mLbo);
}

void balls::initBalls()
{
    float xs = -mBorder.x / 2 + mRadius + 0.05;
    float ys = -mBorder.y / 2 + mRadius + 0.05;
    float d = 2 * mRadius + 0.05;
    float k = mBorder.x / d;

    for (unsigned i = 0; i < mCount; i++) {
        mLoc[i] = glm::vec2(xs + (i % (int)k) * d, ys + (i / (int)k) * d);
        mSpeed[i] = glm::vec2(
            glm::linearRand(-.001f, .001f), glm::linearRand(-.001f, .001f));
    }
    mSpeed[mCount - 1] = glm::vec2(10, 5);
    memcpy(&lbp->speed[1], &lbp->speed[0], sizeof(lbp->speed[0]));
    memcpy(&lbp->loc[1], &lbp->loc[0], sizeof(lbp->loc[0]));
}

void balls::step(float dt)
{
    cindex = 1 - cindex;
    mComputeShader.Bind();
    glUniform1i(1, cindex);
    glUniform1f(4, dt);
    glDispatchCompute(1, 1, 1);
}

float balls::calculateTotalEnergy()
{
    // to be implemented
    float total_energy = 0;
    for (unsigned i = 0; i < mCount; i++) {
        total_energy += (glm::length(mSpeed[i])) * (glm::length(mSpeed[i]));
    }
    return total_energy;
}

void balls::setRadius(float r)
{
    mRadius = r;
    mShader.Bind();
    glUniform1f(2, mRadius);
    mComputeShader.Bind();
    glUniform1f(3, mRadius);
}
