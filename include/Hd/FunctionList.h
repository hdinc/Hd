#pragma once

#include <stdio.h>
#include <functional>
#include <algorithm>
#include <unordered_map>

namespace Hd {

template <typename R, typename... Args>
class FunctionList {

public:
    int add(std::function<R(Args...)> f)
    {
        mFunctions.insert({++id, f});
        return id;
    }

    void remove(int id)
    {
        auto it = mFunctions.find(id);

        if (it != mFunctions.end()) {
            mFunctions.erase(it);
            return;
        }

        printf("[FunctionList]: wrong function id\n");
    }

    void run(Args... args)
    {
        for (auto f : mFunctions) {
            f.second(args...);
        }
    }

private:
    std::unordered_map<int, std::function<R(Args...)>> mFunctions;
    int id = 0;
};

}
