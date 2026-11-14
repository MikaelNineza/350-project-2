#pragma once

#include <unordered_map>

#include "MathUtil.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

namespace ECSEngine {

/// Owns the game window and the camera that maps world space to window space.
///
/// In an ECSEngine, the engine owns the single WindowManager. Get it from
/// EngineView::GetWindowManager(). It lives exactly as long as the engine.
///
/// Coordinate systems:
///  - Window (screen) space: pixels, origin at the window's top-left, +y down.
///  - World space: game units, +y down. The camera is described by the world
///    point at the centre of the window (GetWorldCenter) and the number of
///    world units per window pixel (GetWorldScale; 1.0 = no zoom).
///  All Rect/Point2D arguments below say which space they are in.
///
/// All camera setters take effect immediately for later conversions. Nothing
/// is cached, so changing the camera mid-frame changes how the rest of the
/// frame is drawn.
class WindowManager {
public:
    /// Opens a window of `width` x `height` pixels with the given title.
    /// The camera starts centred on (width/2, height/2) with scale 1, so at
    /// first world space and window space are the same.
    WindowManager(unsigned int width, unsigned int height, const std::string& title);
    ~WindowManager() = default;

    /// Shared handle to the SFML window.
    ///
    /// Ownership is shared: the returned shared_ptr keeps the sf::RenderWindow
    /// object alive even after the WindowManager is destroyed. However, the
    /// engine CLOSES the window in ~ECSEngine(), so a handle that outlives the
    /// engine refers to a closed window and is only useful for querying it.
    /// Systems should take a fresh copy in each Run() (as SpriteSystem does)
    /// rather than storing one.
    std::shared_ptr<sf::RenderWindow> GetWindow() const;

    /// The window's area in window space: {(0,0), width, height}. Returned by value.
    Rect GetWindowBounds() const;

    /// Moves the camera so that world point `worldPt` appears at window
    /// point `screenPt`. The scale is unchanged.
    void SetCamera(const Point2D& worldPt, const Point2D& screenPt);
    /// Moves the camera so that `worldCenter` (world space) is at the window centre.
    void SetWorldCenter(const Point2D& worldCenter);
    /// Fits the camera to `worldCameraRect` (world space). The rect's centre
    /// becomes the window centre, and its width fills the window width. The
    /// visible height follows from the window's aspect ratio, not from
    /// worldCameraRect.height.
    void SetWorldRect(const Rect& worldCameraRect);
    /// Sets the zoom in world units per window pixel. Must be > 0.
    void SetWorldScale(float worldUnitsPerPixel);
    /// World point currently at the window centre.
    Point2D GetWorldCenter() const;
    /// Current world units per window pixel.
    float GetWorldScale() const;

    /// Converts a length (not a position) between the two spaces.
    float WindowToWorld(float x) const;
    float WorldToWindow(float x) const;
    /// Converts rects and points between the two spaces using the current camera.
    /// All return by value.
    Rect WindowToWorld(const Rect& rect) const;
    Rect WorldToWindow(const Rect& rect) const;
    Point2D WorldToWindow(const Point2D& pt) const;
    Point2D WindowToWorld(const Point2D& pt) const;

    /// Draws a filled (DebugDrawRect) or outlined (DebugFrameRect) rectangle.
    /// `r` is in WORLD space and is converted using the current camera.
    /// Colour components are 0-255. The shape goes into the window's back
    /// buffer, so it only appears when the window is next displayed
    /// (SpriteSystem calls clear() then display() every frame, so call these
    /// between those two points).
    void DebugDrawRect(const Rect& r, int red, int green, int blue, int alpha = 255);
    void DebugFrameRect(const Rect& r, int red, int green, int blue, int alpha = 255);

    /// Current window size in pixels as (width, height).
    Point2D GetWindowSize() const;

private:
    std::shared_ptr<sf::RenderWindow> mWindow;
    Point2D mViewCenterWorld;   // [world] point at the window centre
    float mWorldUnitsPerPixel;  // zoom: world units per window pixel
};

}  // namespace ECSEngine
