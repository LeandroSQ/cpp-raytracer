#include "engine.hpp"

int main() {
    Engine engine;
    if (engine.init() < 0) return -1;
    engine.loop();

    return 0;
}