#include <vector>
#include <utility>
#include <string>
#include <limits>
#include <cmath>
#include <cstdint>

#include "MathUtil.h"
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
		ResolveDynamics(engine);

		// remove objects (stars that player consume)
		for (auto id : mRemove) {
			em.RemoveEntity(id);
		}

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
	struct AABB {
		float left;
		float top;
		float right;
		float bottom;

		float CenterX() {
			return (left + right) * 0.5f;
		}

		float CenterY() {
			return (top + bottom) * 0.5f;
		}

		AABB Offset(float dx, float dy) {
			return { left + dx, top + dy, right + dx, bottom + dy };
		}
	};

	void ResolveDynamics(tEngineView* engine) {
		auto &em = engine->GetEntityManager();
		for (size_t i = 0; i < mDynamics.size(); i++) {
			for (size_t j = i + 1; j < mDynamics.size(); j++) {
				EntityID a = mDynamics[i];
				EntityID b = mDynamics[j];
				AABB aBox = WorldBox(em, a);
				AABB bBox = WorldBox(em, b);
				if (!Overlaps(aBox, bBox)) {
					continue;
				}

				bool aIsPlayer = em.template HasComponent<InputComponent>(a);
				bool bIsPlayer = em.template HasComponent<InputComponent>(b);

				// player touched star
				if (aIsPlayer || bIsPlayer) {
					if (aIsPlayer) {
						mRemove.push_back(b);
					}
					else {
						mRemove.push_back(a);
					}

					auto &sc = em.template GetComponent<ScoreComponent>(mScoreEntity);
					sc.score++;
					engine->GetSoundManager().GetSound("collect star").play();
				}

				// star touched star
				else {
					auto &movA = em.template GetComponent<MovementComponent>(a);
					auto &movB = em.template GetComponent<MovementComponent>(b);
					// stars move away from each other horizontally when colliding
					float dir = (aBox.CenterX() < bBox.CenterX()) ? -1.f : 1.f;
					movA.velocity.x = dir * std::abs(movA.velocity.x);
					movB.velocity.x = -dir * std::abs(movB.velocity.x);
				}
			}
		}
	}

	void ResolveStatics(tEngineView* engine, EntityID id) {
		auto &em = engine->GetEntityManager();
		auto &loc = em.template GetComponent<LocationComponent>(id);
		auto &mov = em.template GetComponent<MovementComponent>(id);
		auto &col = em.template GetComponent<CollisionComponent>(id);
		bool isPlayer = em.template HasComponent<InputComponent>(id);

		uint8_t prevOnGround = col.onGround;
		uint8_t prevOnCeiling = col.onCeiling;
		uint8_t prevOnLeftWall = col.onLeftWall;
		uint8_t prevOnRightWall = col.onRightWall;
		
		float xVelo = mov.velocity.x;
		float yVelo = mov.velocity.y;

		col.ClearSides();

		bool takeDamage = false;

		// previous collision box as AABB
		AABB prevBounds = RectToAABB(col.prevCollisionBox);

		// current collision box as AABB (location added)
		AABB currBounds = WorldBox(em, id);

		float xMove = currBounds.left - prevBounds.left;
		float yMove = currBounds.top - prevBounds.top;

		auto applyContact = [&](Side side, bool doesDamage) {
			// mark which sides have been collided with
			// helpers for horizontal/vertical collision will return None if there was no collision
			switch (side) {
				case Side::None:
					return;
				case Side::Left:
					col.onLeftWall = true;
					break;
				case Side::Right:
					col.onRightWall = true;
					break;
				case Side::Top:
					col.onCeiling = true;
					break;
				case Side::Bottom:
					col.onGround = true;
					break;
			}

			takeDamage = takeDamage || doesDamage;

			bool vertical = (side == Side::Top) || (side == Side::Bottom);
			float &normal = vertical ? mov.velocity.y : mov.velocity.x;
			float &tangent = vertical ? mov.velocity.x : mov.velocity.y;

			// check if the collision occurred because of direction of movement
			// if it did, it means the velocity in that direction needs to be reset/changed
			// if not (it's already moving away), dont do anything 
			bool movingInto = (side == Side::Bottom && normal) > 0.f ||
						   (side == Side::Top && normal < 0.f) ||
					   (side == Side::Right && normal > 0.f) ||
					   (side == Side::Left && normal < 0.f);

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
				if (std::abs(normal) < 1.f) {
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
			auto doesDamage = em.template GetComponent<CollisionComponent>(tile).doesDamage;
			applyContact(VerticalStep(prevBounds, box, tileBox), doesDamage);
		}

		// horizontal movement
		box = box.Offset(xMove, 0.f);
		for (EntityID tile : mStatics) {
			auto tileBox = WorldBox(em, tile);
			auto doesDamage = em.template GetComponent<CollisionComponent>(tile).doesDamage;
			applyContact(HorizontalStep(prevBounds, box, tileBox), doesDamage);
		}

		// correct the location of the object based on the offset of the current position to the new box
		loc.loc.x += box.left - currBounds.left;
		loc.loc.y += box.top - currBounds.top;

		if (!isPlayer) {
			return;
		}

		// player died, respawn
		if (takeDamage && em.template HasComponent<RespawnComponent>(id)) {
			auto &respawn = em.template GetComponent<RespawnComponent>(id);
			loc.loc = respawn.loc;
			mov.velocity = ECSEngine::Point2D(0.f, 0.f);
			col.ClearSides();
			col.wallSliding = false;
			return;
		}

		// shake camera based on where player collided
		// player landed (also add sound)
		if (col.onGround && !prevOnGround) {
			engine->GetSoundManager().GetSound("landing sound").play();
			Shake(em, 0.f, yVelo);
		}

		// player hit head
		if (col.onCeiling && !prevOnCeiling) {
			Shake(em, 0.f, yVelo);
		}

		// player hit wall (left or right)
		if ((col.onLeftWall && !prevOnLeftWall) || (col.onRightWall && !prevOnRightWall)) {
			Shake(em, xVelo, 0.f);
		}
	}

	static AABB RectToAABB(ECSEngine::Rect &r) {
		return { r.topLeft.x, r.topLeft.y, r.topLeft.x + r.width, r.topLeft.y + r.height };
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
		return RectToAABB(col.collisionBox).Offset(loc.loc.x, loc.loc.y);
	}

	static bool Overlaps(AABB &a, AABB &b) {
		return a.left < b.right && b.left < a.right && a.top < b.bottom && b.top < a.bottom;
	}

	void Shake(tEntityManager &em, float xVelo, float yVelo) {
		auto &shake = em.template GetComponent<CameraShakeComponent>(mShakeEntity);
		shake.frameLimit = 6; // change later / test values
		float shakeScale = 1.f / 60.f; // change later / test values
		shake.offset.x = xVelo * shakeScale;
		shake.offset.y = yVelo * shakeScale;
	}

	// collect all objects relevant for the collision system
	// empty slots are implicitly ignored since they have no components
	void GetEntities(tEntityManager &em) {
		mStatics.clear();
		mDynamics.clear();
		mRemove.clear();

		for (const auto &entity : em) {
			EntityID id = entity.GetID();
			if (em.template HasComponent<CameraShakeComponent>(id)) {
				mShakeEntity = id;
			}
			if (em.template HasComponent<ScoreComponent>(id)) {
				mScoreEntity = id;
			}

			if (!em.template HasComponent<CollisionComponent>(id)) {
				continue;
			}

			auto &col = em.template GetComponent<CollisionComponent>(id);
			if (col.staticEntity) {
				mStatics.push_back(id);
			}
			else if (em.template HasComponent<MovementComponent>(id)) {
				mDynamics.push_back(id);
			}
		}
	}

	std::string mName = "CollisionSystem";
	std::vector<EntityID> mStatics;
	std::vector<EntityID> mDynamics;
	std::vector<EntityID> mRemove;
	EntityID mShakeEntity;
	EntityID mScoreEntity;
};
