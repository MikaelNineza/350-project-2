#pragma once

#include <string>
#include <algorithm>

#include "Entity.h"
#include "LocationComponent.h"
#include "CollisionComponent.h"
#include "GravityComponent.h"
#include "InputComponent.h"
#include "ECSEngine.h"
#include "MovementComponent.h"

namespace ECSEngine
{

template <typename... Components>
class GravitySystem : public System<Components...> {
public:
	const std::string & GetName() const override {
		static std::string name("GravitySystem");
		return name;
	}
	
	tSystemReturn Run(EngineView<Components...> *engine) override
	{
		auto &em = engine->GetEntityManager();
		for (auto &entity : em) {
			EntityID id = entity.GetID();
			if (!em.template HasComponent<GravityComponent>(id) ||
				!em.template HasComponent<MovementComponent>(id)) {
				continue;
			}

			auto &gravity = em.template GetComponent<GravityComponent>(id);
			auto &mov = em.template GetComponent<MovementComponent>(id);

			if (gravity.disabledTimer > 0) {
				gravity.disabledTimer--;
				continue;
			}

			bool player = em.template HasComponent<InputComponent>(id);

			// player will fall faster
			auto acceleration = gravity.acceleration;
			if (player && mov.velocity.y > 0.f) {
				acceleration.y *= multiplier; // change later
			}

			mov.velocity = mov.velocity + acceleration / 60.f;
			mov.velocity.y = std::min(mov.velocity.y, maxSpeed);

			if (em.template HasComponent<CollisionComponent>(id)) {
				auto &col = em.template GetComponent<CollisionComponent>(id);
				if (col.onLeftWall || col.onRightWall) {
					mov.velocity.y = std::min(mov.velocity.y, maxSpeedSliding);
				}
			}
		}
		return kContinue;
	}
private:
	// test values
	// change depending on jump speed
	static constexpr float multiplier = 1.5f;
	static constexpr float maxSpeed = 600.f;
	static constexpr float maxSpeedSliding = 60.f;
};


}



