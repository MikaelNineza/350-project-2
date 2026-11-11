#include "WindowManager.h"

namespace ECSEngine {

WindowManager::WindowManager(unsigned int width, unsigned int height, const std::string& title)
    : mWindow(std::make_shared<sf::RenderWindow>(sf::VideoMode({width, height}), title)), mViewCenterWorld(static_cast<float>(width) / 2.f, static_cast<float>(height) / 2.f), mWorldUnitsPerPixel(1.f) {}

std::shared_ptr<sf::RenderWindow> WindowManager::GetWindow() const { return mWindow; }

Rect WindowManager::GetWindowBounds() const {
    return Rect(Point2D(0.f, 0.f), GetWindowSize().x, GetWindowSize().y);
}

void WindowManager::DebugDrawRect(const Rect& r, int red, int green, int blue, int alpha) {
    Rect r2 = WorldToWindow(r);
    sf::RectangleShape rectangle({r2.width, r2.height});
    rectangle.setPosition({r2.topLeft.x, r2.topLeft.y});
    rectangle.setFillColor(sf::Color(red, green, blue, alpha));
    mWindow->draw(rectangle);
}

void WindowManager::DebugFrameRect(const Rect& r, int red, int green, int blue, int alpha) {
    Rect r2 = WorldToWindow(r);
    sf::RectangleShape rectangle({r2.width, r2.height});
    rectangle.setPosition({r2.topLeft.x, r2.topLeft.y});
    rectangle.setOutlineColor(sf::Color(red, green, blue, alpha));
    rectangle.setFillColor(sf::Color::Transparent);
    rectangle.setOutlineThickness(2.f);
    mWindow->draw(rectangle);
}

Rect WindowManager::WindowToWorld(const Rect& rect) const {
    return Rect{WindowToWorld(rect.topLeft), WindowToWorld(rect.width), WindowToWorld(rect.height)};
}

Point2D WindowManager::WindowToWorld(const Point2D& pt) const {
    return mViewCenterWorld - ((GetWindowSize() * 0.5f) * mWorldUnitsPerPixel) +
               (pt * mWorldUnitsPerPixel);
}

Rect WindowManager::WorldToWindow(const Rect& rect) const {
    return Rect{WorldToWindow(rect.topLeft), WorldToWindow(rect.width), WorldToWindow(rect.height)};
}

Point2D WindowManager::WorldToWindow(const Point2D& pt) const {
    return (pt - (mViewCenterWorld - ((GetWindowSize() * 0.5f) * mWorldUnitsPerPixel))) /
               mWorldUnitsPerPixel;
}

void WindowManager::SetCamera(const Point2D& worldPt, const Point2D& screenPt) {
    mViewCenterWorld = worldPt - screenPt * mWorldUnitsPerPixel + GetWindowSize() * 0.5f * mWorldUnitsPerPixel;
}

Point2D WindowManager::GetWorldCenter() const {
    return mViewCenterWorld;
}

float WindowManager::GetWorldScale() const {
    return mWorldUnitsPerPixel;
}

void WindowManager::SetWorldCenter(const Point2D& worldCenter) {
    mViewCenterWorld = worldCenter;
}

void WindowManager::SetWorldScale(float worldUnitsPerPixel) {
    mWorldUnitsPerPixel = worldUnitsPerPixel;
}

void WindowManager::SetWorldRect(const Rect& worldCameraRect) {
    mViewCenterWorld = worldCameraRect.topLeft + Point2D(worldCameraRect.width * 0.5f, worldCameraRect.height * 0.5f);
    mWorldUnitsPerPixel = worldCameraRect.width / mWindow->getSize().x; 
}

float WindowManager::WindowToWorld(float x) const {
    return x * mWorldUnitsPerPixel;
}

float WindowManager::WorldToWindow(float x) const {
    return x / mWorldUnitsPerPixel;
}

Point2D WindowManager::GetWindowSize() const {
    return Point2D(static_cast<float>(mWindow->getSize().x), static_cast<float>(mWindow->getSize().y));
}
}  // namespace ECSEngine
