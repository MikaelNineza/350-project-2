#pragma once

#include <string>
#include <algorithm>

#include "CollisionComponent.h"
#include "Entity.h"
#include "ECSEngine.h"
#include "Keyboard.h"
#include "InputComponent.h"
#include "LocationComponent.h"

namespace ECSEngine {

const float kMaxRunVelocity = 15;
const float kDashVelocity = 50;
const float kJumpVelocity = 25;
const float kGroundRunAcceleration = 5;
const float kAirRunAcceleration = 4;
const float kDashInterval = 25;
const float kWallJumpVelocityY = 5;
const float kWallJumpVelocityX = 20;
const float kJumpReleaseAcceleration = 5;
const float kDownHeldFallAcceleration = 5;

template <typename... Components>
class InputSystem : public System<Components...> {
public:
    const std::string& GetName() const override {
        static std::string name("InputSystem");
        return name;
    }

    tSystemReturn Run(EngineView<Components...>* engine) override {
        auto &em = engine->GetEntityManager();
        EntityID player;
        for (auto &entity : em) {
            EntityID id = entity.GetID();
            if (!em.template HasComponent<InputComponent>(id)) {
                continue;
            }

            auto &input = em.template GetComponent<InputComponent>(id);
            auto &mov = em.template GetComponent<MovementComponent>(id);
            auto &col = em.template GetComponent<CollisionComponent>(id);

            bool right = input.keydown[Key::Right];
            bool left = input.keydown[Key::Left];
            bool down = input.keydown[Key::Down];
            bool jumping = input.keydown[Key::Z];
            bool jump = input.keydown[Key::Z] && !input.lastKeydown[Key::Z];
            bool dash = input.keydown[Key::C] && !input.lastKeydown[Key::C];

            // adjust speed in left/right direction
            float target = 0.f;
            if (right) {
                target += kMaxRunVelocity;
            }
            if (left) {
                target -= kMaxRunVelocity;
            }

            float step = col.onGround ? kGroundRunAcceleration : kAirRunAcceleration;

            // movement in either direction is bounded by max run velo in that direction
            // adjust x velocity moving right
            if (mov.velocity.x < target) {
                mov.velocity.x = std::min(mov.velocity.x + step, target);
            }
            // adjust x velocity moving left
            else if (mov.velocity.x > target) {
                mov.velocity.x = std::max(mov.velocity.x - step, target);
            }

            if (jump) {
                bool onWall = col.onLeftWall || col.onRightWall;
                bool falling = mov.velocity.y > 0.f;
                // normal jump
                if (col.onGround) {
                    mov.velocity.y = -kJumpVelocity;
                    engine->GetSoundManager().GetSound("jump").play();
                }
                // jumping while on a wall
                else if (onWall && falling) {
                    mov.velocity.y = -kWallJumpVelocityY;
                    mov.velocity.x = col.onLeftWall ? kWallJumpVelocityX : -kWallJumpVelocityX; // go the opposite direction to the wall
                    engine->GetSoundManager().GetSound("jump").play();
                }
            }

            // extra gravity from letting go of jump
            if (mov.velocity.y < 0.f && !jumping) {
                mov.velocity.y += kJumpReleaseAcceleration;
            }

            // extra gravity of pressing down
            if (mov.velocity.y > 0.f && down) {
                mov.velocity.y += kDownHeldFallAcceleration;
            }

            if (input.lastDash > 0) {
                input.lastDash--;
            }

            if (dash && input.lastDash == 0) {
                float dir = 0.f;
                // dash outward if on a wall
                if (col.onLeftWall && !col.onGround) {
                    dir = 1.f;
                }
                else if (col.onRightWall && !col.onGround) {
                    dir = -1.f;
                }
                // dash in held direction if on the ground
                else if (left ^ right) {
                    dir = left ? -1.f : 1.f;
                }
                if (dir != 0.f) {
                    mov.velocity.x = dir * kDashVelocity;
                }
                input.lastDash = kDashInterval;
            }

            input.lastKeydown = input.keydown;
        }

        return kContinue;
    }
};

}  // namespace ECSEngine
