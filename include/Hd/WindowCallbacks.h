#pragma once

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <functional>
#include <vector>
#include <algorithm>

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
        auto it = std::find_if(mFunctions.begin(), mFunctions.end(),
            [id](const decltype(mFunctions[0])& i) { return i.first == id; });

        if (it != mFunctions.end())
            mFunctions.erase(it);
        else
            printf("[WINDOW]: wrong callback id\n");
    }

private:
    std::vector<std::pair<int, std::function<T>>> mFunctions;
    int id = 0;

    friend class Window;
};

}
