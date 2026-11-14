#pragma once

#include <vector>
#include <unordered_map>

#include "MathUtil.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace ECSEngine {

/// Handle to a sprite registered with a SpriteManager.
/// It is a plain index, valid for the lifetime of the SpriteManager that
/// issued it (sprites are never unregistered), and meaningless to any other
/// SpriteManager. It is safe to store in components (SpriteComponent::textureID).
using SpriteID = size_t;

/// Owns all textures (images loaded from disk) and sprites (a texture plus a
/// source sub-rectangle) used for drawing.
///
/// In an ECSEngine, the engine owns the single SpriteManager. Get it from
/// EngineView::GetSpriteManager(). It lives exactly as long as the engine.
///
/// Usage:
///   1. At load time, call RegisterTexture(path, rectInSheet) once per distinct
///      image region and keep the returned SpriteID (usually in a
///      SpriteComponent).
///   2. At draw time, call GetSprite(id), set its position/scale, and draw it
///      immediately (SpriteSystem does this).
///
/// Ownership / lifetime terms:
///  - Textures are cached by path. Each file is loaded from disk at most once
///    and is kept in memory until the manager is destroyed. There is no way
///    to unload a texture.
///  - Sprites are never removed. A SpriteID stays valid until the manager is
///    destroyed.
///  - Every sf::Sprite keeps a non-owning pointer to its sf::Texture.
///    Textures are stored in an unordered_map, so their addresses stay the
///    same as more textures are added, and they are destroyed after the
///    sprites (member order below). Do not reorder the members.
///  - Do not keep any sf::Sprite or sf::Texture from this manager after the
///    engine is destroyed.
class SpriteManager {
public:
    SpriteManager() = default;
    ~SpriteManager() = default;

    /// Registers a sprite that shows `sourceRect` of the image at `texturePath`.
    ///
    /// @param texturePath  Image file path. The first call with a given path
    ///                     loads it from disk, and later calls with the same
    ///                     string reuse the cached texture. Paths are
    ///                     compared as strings, so two spellings of the same
    ///                     file load it twice.
    /// @param sourceRect   Region of the image in pixels (top-left, width,
    ///                     height). Values are truncated to integers.
    /// @return             A new SpriteID. Every call creates a new sprite,
    ///                     even when the path and rect are identical.
    ///
    /// Errors: if the file cannot be loaded, SFML throws sf::Exception, and
    /// nothing is added to the manager.
    ///
    /// Invalidates: any sf::Sprite& previously returned by GetSprite(), because
    /// the sprite vector may reallocate. SpriteIDs are NOT invalidated.
    /// Register everything during setup, before the game loop starts.
    [[nodiscard]] SpriteID RegisterTexture(const std::string& texturePath, const Rect& sourceRect);

    /// Returns the sprite for `id`. The manager still owns it.
    ///
    /// Precondition: `id` was returned by RegisterTexture() on this manager
    /// (asserted).
    ///
    /// The sprite is SHARED: every entity whose SpriteComponent uses this id
    /// draws the same sf::Sprite object. Position/scale you set on it persist
    /// until someone else changes them, so always set the transform right
    /// before drawing, and do not rely on it afterwards.
    ///
    /// The reference is valid until the next RegisterTexture() call or the
    /// destruction of the manager, whichever comes first. Use it in the
    /// current block only and do not store it. Store the SpriteID instead.
    sf::Sprite& GetSprite(SpriteID id);

private:
    // Declaration order matters: mSprites is destroyed first, so no sprite
    // ever outlives the texture it points to.
    std::unordered_map<std::string, sf::Texture> mTextures;  // keyed by file path
    std::vector<sf::Sprite> mSprites;                        // indexed by SpriteID
};

}  // namespace ECSEngine
