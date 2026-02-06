#include "te_input/Touch.hpp"
#include <algorithm>

namespace te_input {

int TouchInput::get_touch_count() const {
    return static_cast<int>(touches_.size());
}

TouchState TouchInput::get_touch(int touch_id) const {
    auto it = std::find_if(touches_.begin(), touches_.end(),
        [touch_id](TouchState const& t) { return t.touch_id == touch_id; });
    if (it != touches_.end())
        return *it;
    TouchState empty{};
    empty.touch_id = touch_id;
    empty.phase = TouchPhase::End;
    return empty;
}

void TouchInput::tick() {
    // Per-frame update from platform touch events; touch_id stable from Begin to End.
}

} // namespace te_input
