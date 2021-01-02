#include "balls.h"
#include <glm/common.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>

const int balls::mCircleVertexCount = 20;

static glm::vec2 rotate90(glm::vec2 x)
{
    glm::vec2 r;

    r.x = -x.y;
    r.y = x.x;

    return r;
}

balls::balls(glm::mat4& projection, int count, float radius, glm::vec2 border)
    : mProjection(projection)
    , mShader("../res/shaders/balls.vert", "../res/shaders/balls.frag")
    , mBorder(border)
{
    mCount = count;
    mRadius = radius;
    mLoc = new glm::vec2[mCount];
    mSpeed = new glm::vec2[mCount];

    initGL();
    initBalls();
}

balls::~balls()
{
    delete[] mLoc;
    delete[] mSpeed;
}

void balls::draw()
{
    // todo: instanced rendering
    // create locations buffer
    // upload it to the gpu
    // call instanced rendering

    mShader.Bind();
    glBindVertexArray(mVao);
    glBindBuffer(GL_ARRAY_BUFFER, mVbo);

    if (mChanged) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, mLbo);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, mCount * sizeof(glm::vec2),
            glm::value_ptr(mLoc[0]));
        mChanged = false;
    }
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, mCircleVertexCount, mCount);
}

void balls::initGL()
{
    setRadius(mRadius);

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
    glBufferData(GL_SHADER_STORAGE_BUFFER, mCount * sizeof(glm::vec2), 0,
        GL_DYNAMIC_DRAW);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, mLbo);
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
}

void balls::step(float dt)
{
    for (unsigned i = 0; i < mCount; i++) {
        mLoc[i] += mSpeed[i] * dt;
    }

    borderBounce();
    ballBounce();
    mChanged = true;
}

void balls::borderBounce()
{
    for (unsigned i = 0; i < mCount; i++) {
        if ((mLoc[i].x > mBorder.x / 2 - balls::mRadius && mSpeed[i].x > 0)
            || (mLoc[i].x < -(mBorder.x / 2 - balls::mRadius)
                && mSpeed[i].x < 0)) {
            mSpeed[i].x = -mSpeed[i].x;
        }
        if ((mLoc[i].y > mBorder.y / 2 - balls::mRadius && mSpeed[i].y > 0)
            || (mLoc[i].y < -(mBorder.y / 2 - balls::mRadius)
                && mSpeed[i].y < 0)) {
            mSpeed[i].y = -mSpeed[i].y;
        }
    }
}

void balls::ballBounce()
{
    glm::vec2 ballAx, ballAy, ballBx, ballBy, AB, nAB;
    for (unsigned i = 0; i < mCount - 1; i++) {
        for (unsigned j = i + 1; j < mCount; j++) {
            if (glm::length(mLoc[i] - mLoc[j]) < 2 * balls::mRadius) {

                AB = glm::normalize(mLoc[j] - mLoc[i]);
                ballAx = AB * glm::dot(mSpeed[i], AB);
                ballBx = AB * glm::dot(mSpeed[j], AB);
                nAB = rotate90(AB);
                ballAy = nAB * glm::dot(mSpeed[i], nAB);
                ballBy = nAB * glm::dot(mSpeed[j], nAB);

                if (glm::dot(ballAx, ballBx) > 0) {
                    if (glm::dot(AB, ballAx) > 0) {
                        if (glm::length(ballAx) > glm::length(ballBx)) {
                            mSpeed[i] = ballBx + ballAy;
                            mSpeed[j] = ballAx + ballBy;
                        }
                    } else {
                        if (glm::length(ballBx) > glm::length(ballAx)) {
                            mSpeed[i] = ballBx + ballAy;
                            mSpeed[j] = ballAx + ballBy;
                        }
                    }
                } else if (glm::dot(ballAx, AB) > 0) {
                    mSpeed[i] = ballBx + ballAy;
                    mSpeed[j] = ballAx + ballBy;
                }
            }
        }
    }
}

float balls::calculateTotalEnergy()
{
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
}
