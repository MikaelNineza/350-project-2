#include <vector>
#include <utility>
#include <string>
#include <limits>
#include <cmath>

#include "ECSEngine.h"
#include "CollisionComponent.h"
#include "MovementComponent.h"
#include "LocationComponent.h"
#include "InputComponent.h"
#include "CameraShakeComponent.h"
#include "ScoreComponent.h"
#include "RespawnComponent.h"

template <typename... Components>
class CollisionSystem : public ECSEngine::System<Components...> {
public:
	ECSEngine::tSystemReturn Run(ECSEngine::EngineView<Components...>* engine) override {
		auto &em = engine->GetEntityManager();
		GetEntities(em);

		// collisions with static objects
		for (auto id : mDynamics) {
			ResolveStatics(engine, id);
		}

		// collisions with dynamic objects

		// remove objects (stars that player consume)

		return ECSEngine::kContinue;
	}

	const std::string &GetName() const override {
		return mName;
	}
private:
	using tEntityManager = ECSEngine::EntityManager<Components...>;
	using tEngineView = ECSEngine::EngineView<Components...>;
	using EntityID = ECSEngine::EntityID;

	enum class Side { None, Top, Bottom, Left, Right };

	void ResolveStatics(tEngineView* engine, EntityID id) {
		auto &em = engine->GetEntityManager();
		auto &loc = em.template GetComponent<LocationComponent>(id);
		auto &mov = em.template GetComponent<MovementComponent>(id);
		auto &col = em.template GetComponent<CollisionComponent>(id);
		bool isPlayer = em.template HasComponent<InputComponent>(id);

		// save all prev locations
		bool prevBottom = col.bottom;
		bool prevTop = col.top;
		bool prevLeft = col.left;
		bool prevRight = col.right;
		
		float xVelo = mov.vx;
		float yVelo = mov.vy;

		col.ClearSides();

		bool touchedHazard = false;

		AABB prevBounds = col.prevBounds;
		AABB currBounds = WorldBox(em, id);
		float xMove = currBounds.left - prevBounds.left;
		float yMove = currBounds.top - prevBounds.top;

		auto applyContact = [&](Side side, bool hazard) {
			// mark which sides have been collided with
			// helpers for horizontal/vertical collision will return None if there was no collision
			switch (side) {
				case Side::None:
					return;
				case Side::Left:
					col.left = true;
					break;
				case Side::Right:
					col.right = true;
					break;
				case Side::Top:
					col.top = true;
					break;
				case Side::Bottom:
					col.bottom = true;
					break;
			}

			touchedHazard = touchedHazard || hazard;

			bool vertical = (side == Side::Top) || (side == Side::Bottom);
			float &normal = vertical ? mov.vy : mov.vx;
			float &tangent = vertical ? mov.vx : mov.vy;

			// check if the collision occurred because of direction of movement
			// if it did, it means the velocity in that direction needs to be reset/changed
			// if not (it's already moving away), dont do anything 
			bool movingInto = (side == Side::Bottom && normal > 0.f ||
						   side == Side::Top && normal < 0.f ||
					   side == Side::Right && normal > 0.f ||
					   side == Side::Left && normal < 0.f);

			if (!movingInto) {
				return;
			}

			// for a player colliding with a static, set velocity to 0 (stop moving in that direction)
			// for a star colliding with a static, bounce off (reverse direction) or stop moving if velo is small
			if (isPlayer) {
				normal = 0.f;
			}
			else {
				// bounce physics for star (ai assisted)
				// direction reverses and both velocities dampen by a factor of 10% (change later/experiment with different values)
				normal *= -0.9f;
				if (std::fabs(normal) < 0.5f) {
					normal = 0.f;
				}
				tangent *= 0.9f;
			}
		};

		// set box indicating the new positions AABB
		// vertical movement
		AABB box = prevBounds.Offset(0.f, yMove);
		for (EntityID tile : mStatics) {
			auto tileBox = WorldBox(em, tile);
			auto isHazard = em.template GetComponent<CollisionComponent>(tile).isHazard;
			applyContact(VerticalStep(prevBounds, box, tileBox), isHazard);
		}

		// horizontal movement
		box = box.Offset(xMove, 0.f);
		for (EntityID tile : mStatics) {
			auto tileBox = WorldBox(em, tile);
			auto isHazard = em.template GetComponent<CollisionComponent>(tile).isHazard;
			applyContact(HorizontalStep(prevBounds, box, tileBox), isHazard);
		}

		// correct the location of the object based on the offset of the current position to the new box
		loc.x += box.left - currBounds.left;
		loc.y += box.top - currBounds.top;

		if (!isPlayer) {
			return;
		}

		// player died, respawn
		if (touchedHazard && em.template HasComponent<RespawnComponent>(id)) {
			auto &respawn = em.template GetComponent<RespawnComponent>(id);
			loc.x = respawn.x;
			loc.y = respawn.y;
			mov.vx = 0.f;
			mov.vy = 0.f;
			col.ClearSides();
			col.wallSliding = false;
			return;
		}

		if (col.bottom && !prevBottom) {
			// play thumping sound
			// add shake effect
		}

		if (col.top && !prevTop) {
			// add shake effect
		}
		if ((col.left && !prevLeft) || col.right && !prevRight) {
			// add shake effect
		}

	}

	static Side VerticalStep(AABB &prev, AABB &box, AABB &tile) {
		if (Overlaps(prev, tile) || !Overlaps(box, tile)) {
			return Side::None;
		}
		// y increases as the player goes down
		// colliding tile with greater y means that box landed on top of it
		// move box (subtract from y) above tile
		if (prev.CenterY() < tile.CenterY()) {
			box = box.Offset(0.f, tile.top - box.bottom);
			return Side::Bottom;
		}

		// otherwise, box ceiling hit the bottom of a tile
		// add to y to move below tile
		box = box.Offset(0.f, tile.bottom - box.top);
		return Side::Top;
	}

	static Side HorizontalStep(AABB &prev, AABB &box, AABB &tile) {
		// dont handle horizontal overlap if it was already overlapping or there is no overlap in the new pos
		if (Overlaps(prev, tile) || !Overlaps(box, tile)) {
			return Side::None;
		}

		// the right side hit a tile
		// move box left so that its exactly aligned with the left side of the tile
		if (prev.CenterX() < tile.CenterX()) {
			box = box.Offset(tile.left - box.right, 0.f);
			return Side::Right;
		}

		// the left side hit a tile
		// move box right so that its exactly aligned with the right side of the tile
		box = box.Offset(tile.right - box.left, 0.f);
		return Side::Left;
	}

	static AABB WorldBox(tEntityManager &em, EntityID id) {
		auto &loc = em.template GetComponent<LocationComponent>(id);
		auto &col = em.template GetComponent<CollisionComponent>(id);
		return col.bounds.Offset(loc.x, loc.y);
	}

	static bool Overlaps(AABB &a, AABB &b) {
		return a.left < b.right && b.left < a.right && a.top < b.bottom && b.top < a.bottom;
	}

	// collect all objects relevant for the collision system
	// empty slots are implicitly ignored since they have no components
	void GetEntities(tEntityManager &em) {
		mStatics.clear();
		mDynamics.clear();
		mRemove.clear();

		for (const auto &entity : em) {
			ECSEngine::EntityID id = entity.GetID();
			if (em.template HasComponent<CameraShakeComponent>(id)) {
				mShakeEntity = id;
			}
			if (em.template HasComponent<ScoreComponent>(id)) {
				mScoreEntity = id;
			}

			if (!em.template HasComponent<CollisionComponent>(id) ||
				!em.template HasComponent<LocationComponent>(id)) {
				continue;
				}

			auto &col = em.template GetComponent<CollisionComponent>(id);
			if (col.isStatic) {
				mStatics.push_back(id);
			}
			else if (em.template HasComponent<MovementComponent>(id)) {
				mDynamics.push_back(id);
			}
		}
	}

	std::string mName = "CollisionSystem";
	std::vector<ECSEngine::EntityID> mStatics;
	std::vector<ECSEngine::EntityID> mDynamics;
	std::vector<ECSEngine::EntityID> mRemove;
	ECSEngine::EntityID mShakeEntity;
	ECSEngine::EntityID mScoreEntity;
};
