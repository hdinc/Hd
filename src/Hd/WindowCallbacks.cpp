#include <Hd/WindowCallbacks.h>
#include <Hd/Camera2D.h>
#include <stdio.h>

namespace Hd {

template <typename T>
int Callback<T>::addFunc(T function)
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

template <typename T>
void Callback<T>::removeFunc(int n)
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

template class Callback<void (*)(double, double)>;
template class Callback<void (*)(int, int, int, int)>;
template class Callback<void (*)(int, int, int)>;
template class Callback<void (*)(int, int)>;

}
