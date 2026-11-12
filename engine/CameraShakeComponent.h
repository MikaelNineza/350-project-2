#include "WindowManager.h"
#include "MathUtil.h"

struct CameraShakeComponent {
    int frameLimit;
    ECSEngine::Point2D offset;
};