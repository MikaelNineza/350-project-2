#pragma once

#include "WindowManager.h"
#include "MathUtil.h"

namespace ECSEngine {
    struct CameraShakeComponent {
    int frameLimit;
    ECSEngine::Point2D offset;
};
}