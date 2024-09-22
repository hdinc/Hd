#pragma once

#include <algorithm>
#include <list>
#include <unordered_map>

namespace Hd {

template <typename key, typename val>
class LRUCache {
public:
    LRUCache(int cap);

    val get(key k);
    val getD(key k);

    bool contains(key k);

    void set(key k, val&& v);
    void clear();

private:
    LRUCache(const LRUCache&) = delete;
    LRUCache& operator=(const LRUCache&) = delete;

    void reorder(key k);

    std::unordered_map<key, val> c;
    std::list<key> q;
    int cap;
};

template<typename key, typename val>
LRUCache<key, val>::LRUCache(int cap) : cap{cap} {}

template<typename key, typename val>
val LRUCache<key, val>::get(key k) {
    auto it = c.find(k);
    if (it == c.end()) {
        throw 1;
    }
    reorder(k);
    return it->second;
}

template<typename key, typename val>
val LRUCache<key, val>::getD(key k) {
    auto it = c.find(k);
    if (it == c.end()) {
        throw 1;
    }
    auto r = std::move(it->second);
    c.erase(it);
    q.erase(std::find(q.begin(), q.end(), k));
    return r;
}

template<typename key, typename val>
void LRUCache<key, val>::clear() {
    q.clear();
    c.clear();
}

template<typename key, typename val>
void LRUCache<key, val>::set(key k, val&& v) {
    auto it = c.find(k);
    if (it != c.end()) {
        it->second = std::forward<val>(v);
        reorder(k);
    } else {
        if (c.size() == cap) {
            c.erase(q.back());
            q.pop_back();
        }
        q.push_front(k);
        c.emplace(k, std::forward<val>(v));
    }
}

template<typename key, typename val>
void LRUCache<key, val>::reorder(key k) {
    auto it = std::find(q.begin(), q.end(), k);
    q.erase(it);
    q.push_front(k);
}

template<typename key, typename val>
bool LRUCache<key, val>::contains(key k) {
    return c.find(k) != c.end();
}

}
