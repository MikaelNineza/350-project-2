struct AABB {
	float left = 0.f;
	float right = 0.f;
	float top = 0.f;
	float bottom = 0.f;

	float CenterX() {
		return (left + right) * 0.5f;
	}

	float CenterY {
		return (top + bottom) * 0.5f;
	}

	AABB Offset(float dx, float dy) {
		return {left + dx, top + dy, right + dx, bottom + dy};
	}
};

struct CollisionComponent {
	AABB bounds;		// current bounds in local coordiantes
	AABB prevBbox;		// previous bounds
	bool isStatic = true;	// is static (like a wall) or dynamic (like player)
	bool isHazard = false;  // player dies upon contact with hazardous obj
	bool left = false;	// all side flags indicate which side this object hit
	bool right = false;
	bool top = false;
	bool bottom = false;
	bool sliding = false;	// sliding against a wall and falling
	void ClearSides() {
		left = false;
		right = false;
		top = false;
		bottom = false;
	}
};
