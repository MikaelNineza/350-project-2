#pragma once

#include <bitset>
#include <unordered_map>

#include "MathUtil.h"
#include "SpriteManager.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace ECSEngine {

/// Makes an entity drawable. SpriteSystem draws it only if the entity also
/// has a LocationComponent.
///
/// Stores a SpriteID handle, not the sprite itself. The SpriteManager owns the
/// sprite and texture, and the handle stays valid for the engine's lifetime,
/// so this component can be copied and kept freely. Several entities may
/// share one textureID.
class SpriteComponent {
public:
    SpriteID textureID;  // From SpriteManager::RegisterTexture() on this engine's SpriteManager.
    Rect bounds;      // Draw rect relative to the entity's LocationComponent::loc. The image is
                      // stretched to bounds.width x bounds.height.
    bool worldSpace;  // If false, this sprite is drawn in screen space instead of world space
private:
};

}  // namespace ECSEngine
