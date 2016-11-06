#ifndef UTILS_H
#define UTILS_H

#include <cstdlib> //we need to include something to have size_t

inline size_t masknbit(size_t number, size_t n) {
    //1<<n is undefined when n >= 64
    return (n >= 64) ? number : (number & ((1<<n)-1));
}

#endif

