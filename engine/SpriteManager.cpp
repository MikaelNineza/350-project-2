#include "SpriteManager.h"

namespace ECSEngine {

SpriteID SpriteManager::RegisterTexture(const std::string& texturePath, const Rect& sourceRect) {
    auto it = mTextures.find(texturePath);
    if (it == mTextures.end()) {
        mTextures[texturePath] = sf::Texture{texturePath};
    }

    sf::Sprite sprite(mTextures[texturePath]);
    sf::Vector2i arg1(static_cast<int>(sourceRect.topLeft.x),
                      static_cast<int>(sourceRect.topLeft.y));
    sf::Vector2i arg2(static_cast<int>(sourceRect.width), static_cast<int>(sourceRect.height));

    sprite.setTextureRect(sf::IntRect(arg1, arg2));
    mSprites.emplace_back(sprite);
    return mSprites.size() - 1;
}

sf::Sprite& SpriteManager::GetSprite(SpriteID id) {
    assert(mSprites.size() > id);
    return mSprites[id];
}

}  // namespace ECSEngine
