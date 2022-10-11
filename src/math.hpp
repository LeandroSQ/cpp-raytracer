#pragma once

template <class T>
inline T max(T a, T b) {
    return a > b ? a : b;
}

template <class T>
inline T clamp(T value, T min, T max) {
    return (value > max ? max : (value < min ? min : value));
}