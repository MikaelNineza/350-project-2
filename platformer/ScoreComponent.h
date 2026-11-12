#include <vector>
#include "ECSEngine.h"

struct ScoreComponent {
    int score;
    ECSEngine::SpriteID digitTextureID[10];
    ECSEngine::EntityID digitID[3];
};