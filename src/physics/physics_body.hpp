#include <box2d/box2d.h>

#include "../ecsql/world.hpp"

struct BodyUserData {
	ecsql::EntityID entity_id;
	b2Transform latest_transform;
	b2Transform previous_transform;

	void update_transform(b2Transform transform);
	b2Transform interpolated_transform(float alpha) const;

	static BodyUserData *from(b2BodyId body_id);
};

extern std::unordered_map<ecsql::EntityID, b2BodyId> body_map;

void register_physics_body(ecsql::World& world);
