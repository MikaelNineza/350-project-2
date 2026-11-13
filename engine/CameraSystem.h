#pragma once

#include <unordered_map>

#include "CameraComponent.h"
#include "LocationComponent.h"
#include "MathUtil.h"
#include "SpriteComponent.h"
#include "SystemManager.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace ECSEngine {

template <typename... Components>
class CameraSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name = "CameraSystem";
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        auto &em = engine->GetEntityManager();
        auto &wm = engine->GetWindowManager();
        for (const auto &entity : em) {
            EntityID id = entity.GetID();
            if (!em.template HasComponent<CameraComponent>(id)) {
                continue;
            }
            if (!em.template HasComponent<CameraFollower>(id)) {
                continue;
            }

            EntityID follower = em.template GetComponent<CameraFollower>(id).entityID;

            if (!em.template HasComponent<LocationComponent>(follower)) {
                continue;
            }
            if (!em.template HasComponent<SpriteComponent>(follower)) {
                continue;
            }

            CameraComponent &camera = em.template GetComponent<CameraComponent>(id);
            Point2D location = em.template GetComponent<LocationComponent>(follower).loc;
            Rect spriteBounds = em.template GetComponent<SpriteComponent>(follower).bounds;

            Point2D temp(spriteBounds.width / 2, spriteBounds.height / 2);
            Point2D playerCenterWorld = location + spriteBounds.topLeft + temp;
            wm.SetWorldScale(camera.worldUnitsPerPixel);
            wm.SetWorldCenter(camera.position);
            Point2D playerCenterWindow = wm.WorldToWindow(playerCenterWorld);

            Rect softLimit = camera.softExclusion;
            Rect hardLimit = camera.hardExclusion;
            float playerX = 0;
            float playerY = 0;

            // Obtain change in x
            if (playerCenterWindow.x < hardLimit.topLeft.x) {
                playerX = playerCenterWindow.x - hardLimit.topLeft.x;
            } else if (playerCenterWindow.x > (hardLimit.topLeft.x + hardLimit.width)) {
                playerX = playerCenterWindow.x - (hardLimit.topLeft.x + hardLimit.width);
            } else if (playerCenterWindow.x < softLimit.topLeft.x) {
                playerX = camera.speed * (playerCenterWindow.x - softLimit.topLeft.x);
            } else if (playerCenterWindow.x > (softLimit.topLeft.x + softLimit.width)) {
                playerX = camera.speed * (playerCenterWindow.x - (softLimit.topLeft.x + softLimit.width));
            }

            // Obtain change in y
            if (playerCenterWindow.y < hardLimit.topLeft.y) {
                playerY = playerCenterWindow.y - hardLimit.topLeft.y;
            } else if (playerCenterWindow.y > (hardLimit.topLeft.y + hardLimit.height)) {
                playerY = playerCenterWindow.y - (hardLimit.topLeft.y + hardLimit.height);
            } else if (playerCenterWindow.y < softLimit.topLeft.y) {
                playerY = camera.speed * (playerCenterWindow.y - softLimit.topLeft.y);
            } else if (playerCenterWindow.y > (softLimit.topLeft.y + softLimit.height)) {
                playerY = camera.speed * (playerCenterWindow.y - (softLimit.topLeft.y + softLimit.height));
            }

            camera.position = camera.position + Point2D(playerX, playerY) * camera.worldUnitsPerPixel;
            wm.SetWorldScale(camera.worldUnitsPerPixel);
            wm.SetWorldCenter(camera.position);

        }
        return kContinue;
    }
};

}  // namespace ECSEngine
