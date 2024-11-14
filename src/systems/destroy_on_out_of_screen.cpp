#include <cdedent.hpp>

#include "destroy_on_out_of_screen.hpp"
#include "../ecsql/system.hpp"

void register_destroy_on_out_of_screen(ecsql::World& world) {
	world.register_system({
		"DestroyOnOutOfScreen",
		R"(
			DELETE FROM entity
			WHERE id IN (
				SELECT entity_id
				FROM DestroyOnOutOfScreen
					JOIN Position USING(entity_id)
					JOIN screen_size
				WHERE position.x NOT BETWEEN 0 AND screen_size.width
					OR position.y NOT BETWEEN 0 AND screen_size.height
			)
		)"_dedent,
		[](auto& delete_out_of_screen) {
			delete_out_of_screen();
		}
	});
}
