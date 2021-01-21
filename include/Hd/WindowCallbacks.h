#pragma once

#include <GLFW/glfw3.h>
#include <stdio.h>

namespace Hd {

template <typename T, typename... arg>
class Callback {

public:
    int addFunc(T function)
    {
        if (mCount >= maxcount_) {
            printf("[WINDOW]:max callback count exceeded\n");
            return -1;
        }
        mFunctions[mCount] = function;
        mIds[mCount] = ++mIdCount;
        mCount++;
        return mIdCount;
    }

    void removeFunc(int n)
    {
        for (int i = 0; i < mCount; i++) {
            if (mIds[i] == n) {
                mFunctions[i] = mFunctions[mCount];
                mIds[i] = mIds[mCount];
                mCount--;
                return;
            }
        }
        printf("[WINDOW]:invalid callback id\n");
    }

    void run(arg... args)
    {
        for (int i = 0; i < mCount; i++) {
            mFunctions[i](args...);
        }
    }

private:
    static const int maxcount_ = 20;
    int mCount = 0;
    int mIdCount = 0;
    T mFunctions[maxcount_] = { 0 };
    int mIds[maxcount_] = { 0 };
};

}
