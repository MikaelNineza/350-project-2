#include "MathUtil.h"

struct GravityComponent {
    GravityComponent(Point2D acceleration, int disabledTimer = 0) : acceleration(acceleration), disabledTimer(0) {
    }
    ECSEngine::Point2D acceleration;
    int disabledTimer;
};