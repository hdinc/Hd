#pragma once

#include <stdio.h>
#include <functional>
#include <list>

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
        auto it = mIds.begin();
        auto itf = mFunctions.begin();
        for (; it != mIds.end(); it++, itf++) {
            if (*it == id) {
                mIds.erase(it);
                mFunctions.erase(itf);
                return;
            }
        }
        printf("[FunctionList]: wrong function id\n");
    }

    void run(Args... args)
    {
        for (int i = 0; i < mFunctions.size(); i++) {
            mFunctions[i](args...);
        }
    }

private:
    std::list<std::function<R(Args...)>> mFunctions;
    std::list<int> mIds;
    int id = 0;
};

}
