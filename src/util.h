#ifndef UTIL_H
#define UTIL_H

#include <thread>

inline void delayMicros(int micros) {
    std::this_thread::sleep_for(std::chrono::microseconds(micros));
} 

#endif // UTIL_H