#pragma once

#include <unordered_map>

#include "CameraComponent.h"
#include "CameraShakeComponent.h"
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
            if (!em.ValidEntity(id)) {
					continue;
			}
            if (!em.template HasComponent<CameraComponent>(id)) {
                continue;
            }
            if (!em.template HasComponent<CameraFollower>(id)) {
                continue;
            }

            EntityID follower = em.template GetComponent<CameraFollower>(id).entityID;
            if (!em.ValidEntity(follower)) {
					continue;
			}
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

            camera.position = camera.position + Point2D(playerX, 0.0f) * camera.worldUnitsPerPixel;
            auto viewCenter = camera.position;

            if (em.template HasComponent<CameraShakeComponent>(id)) {
                auto &shake = em.template GetComponent<CameraShakeComponent>(id);
                if (shake.frameLimit > 0) {
                    float sgn = (shake.frameLimit % 2 == 0) ? 1.f : -1.f;
                    viewCenter = viewCenter + sgn * shake.offset;
                    shake.frameLimit--;
                }
            }

            wm.SetWorldScale(camera.worldUnitsPerPixel);
            wm.SetWorldCenter(viewCenter);

        }
        return kContinue;
    }
};

}  // namespace ECSEngine
