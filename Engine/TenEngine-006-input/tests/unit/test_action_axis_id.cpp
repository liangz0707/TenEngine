#include "te_input/ActionId.hpp"
#include "te_input/AxisId.hpp"

int main() {
    auto a1 = te_input::ActionId::from_name("jump");
    auto a2 = te_input::ActionId::from_name("jump");
    auto a3 = te_input::ActionId::from_name("fire");
    if (a1 != a2) return 1;
    if (a1 == a3) return 2;

    auto x1 = te_input::AxisId::from_name("move");
    auto x2 = te_input::AxisId::from_name("move");
    auto x3 = te_input::AxisId::from_name("look");
    if (x1 != x2) return 3;
    if (x1 == x3) return 4;

    return 0;
}
