#pragma once

#include <stdio.h>
#include <functional>
#include <vector>

namespace Hd {

template <typename R, typename... Args>
class FunctionList {

public:
    int add(std::function<R(Args...)> f)
    {
        mFunctions.push_back(f);
        mIds.push_back(++id);
        return id;
    }

    void remove(int id)
    {
        for (int i = 0; i < mIds.size(); i++) {
            if (mIds[i] == id) {
                mIds.erase(mIds.begin() + i);
                mFunctions.erase(mFunctions.begin() + i);
                return;
            }
        }
        printf("[FunctionList]: wrong function id\n");
    }

    void run(Args... args)
    {
        for (auto f : mFunctions) {
            f(args...);
        }
    }

private:
    std::vector<std::function<R(Args...)>> mFunctions;
    std::vector<int> mIds;
    int id = 0;
};

}
