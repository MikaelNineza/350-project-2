#include <cstdint>
#include "MathUtil.h"

struct CollisionComponent {
	CollisionComponent() : staticEntity(0), doesDamage(0), onGround(0), onCeiling(0), onLeftWall(0), onRightWall(0), wallSliding(0) {
	}
	uint8_t staticEntity : 1;
	uint8_t doesDamage : 1;
	uint8_t onGround : 1;
	uint8_t onCeiling : 1;
	uint8_t onLeftWall : 1;
	uint8_t onRightWall : 1;
	uint8_t wallSliding : 1;
	ECSEngine::Rect collisionBox{};
	ECSEngine::Rect prevCollisionBox{};
	void ClearSides() {
		onCeiling = 0;
		onGround = 0;
		onLeftWall = 0;
		onRightWall = 0;
	}
};
