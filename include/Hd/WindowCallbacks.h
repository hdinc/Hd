#pragma once

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <functional>
#include <vector>

namespace Hd {

template <typename T>
class Callback {

public:
    int add(std::function<T> f)
    {
        mFunctions.push_back(std::pair<int, std::function<T>>(++id, f));
        return id;
    }

    void remove(int id)
    {
        for (size_t i = 0; i < mFunctions.size(); i++) {
            if (mFunctions[i].first == id)
                mFunctions.erase(mFunctions.begin() + i);
            return;
        }
    }

private:
    std::vector<std::pair<int, std::function<T>>> mFunctions;
    int id = 0;

    friend class Window;
};

}
