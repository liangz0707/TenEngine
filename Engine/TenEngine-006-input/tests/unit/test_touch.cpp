#include "te_input/Touch.hpp"

int main() {
    te_input::TouchInput touch;
    if (touch.get_touch_count() != 0) return 1;
    te_input::TouchState s = touch.get_touch(0);
    if (s.phase != te_input::TouchPhase::End) return 2;
    touch.tick();
    return 0;
}
