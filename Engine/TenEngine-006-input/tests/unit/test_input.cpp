/**
 * @file test_input.cpp
 * @brief Basic test for Input module using new te::input interface.
 */
#include "te/input/Input.h"
#include "te/input/InputTypes.h"
#include "te/core/log.h"

int main() {
    // Test CreateInput factory function
    auto* input = te::input::CreateInput();
    if (!input) {
        return 1;
    }

    // Test keyboard input (should return false for uninitialized state)
    if (input->GetKey(te::input::KeyCode::Space)) {
        return 2;
    }
    if (input->GetKeyDown(te::input::KeyCode::Space)) {
        return 3;
    }
    if (input->GetKeyUp(te::input::KeyCode::Space)) {
        return 4;
    }

    // Test mouse input
    int32_t x = 0, y = 0;
    input->GetMousePosition(&x, &y);
    if (x != 0 || y != 0) {
        return 5;
    }

    int32_t dx = 0, dy = 0;
    input->GetMouseDelta(&dx, &dy);
    if (dx != 0 || dy != 0) {
        return 6;
    }

    if (input->GetMouseButton(te::input::MouseButton::Left)) {
        return 7;
    }

    // Test gamepad input
    if (input->GetGamepadCount() != 0) {
        return 8;
    }

    // Test touch input
    if (input->GetTouchCount() != 0) {
        return 9;
    }

    // Test input abstraction (Action/Axis)
    te::input::ActionId actionId = 1;
    input->RegisterAction(actionId, "TestAction");
    if (input->GetActionState(actionId)) {
        return 10;
    }

    te::input::AxisId axisId = 1;
    input->RegisterAxis(axisId, "TestAxis");
    if (input->GetAxisValue(axisId) != 0.0f) {
        return 11;
    }

    // Cleanup
    delete input;

    return 0;
}
