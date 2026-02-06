#ifndef TE_INPUT_TOUCH_HPP
#define TE_INPUT_TOUCH_HPP

#include <vector>

namespace te_input {

/// Touch phase (contract-declared).
enum class TouchPhase {
    Begin,
    Move,
    End,
};

/// Touch point state: id stable from Begin to End across frames (contract-declared).
struct TouchState {
    int touch_id{0};
    float x{0.f};
    float y{0.f};
    TouchPhase phase{TouchPhase::End};
};

/// Touch input: multi-touch, get_touch_count / get_touch(id) (contract-declared).
class TouchInput {
public:
    TouchInput() = default;

    int get_touch_count() const;
    TouchState get_touch(int touch_id) const;

    void tick();

private:
    std::vector<TouchState> touches_;
};

} // namespace te_input

#endif
