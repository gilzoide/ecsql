#include <cdedent.hpp>
#include <config.h>
#include <raylib.h>
#include <reflect>

#include "key_handler.hpp"
#include "../ecsql/sql_value.hpp"
#include "../ecsql/system.hpp"

static void IsKeyReleased_function(sqlite3_context *ctx, int argc, sqlite3_value **argv) {
	ecsql::SQLValue key(argv[0]);
	sqlite3_result_int(ctx, IsKeyReleased(key.get_int()));
}

void register_key_handler(ecsql::World& world) {
	{
		ecsql::PreparedSQL insert_key = world.prepare_sql("INSERT INTO Keyboard(key, name) VALUES(?, ?)");
		for (int key = KEY_NULL + 1; key < MAX_KEYBOARD_KEYS; key++) {
			std::string_view enum_name = reflect::enum_name<KeyboardKey, "", 0, MAX_KEYBOARD_KEYS - 1>((KeyboardKey) key);
			if (!enum_name.empty()) {
				insert_key(key, enum_name);
			}
		}
	}

	world.create_function("IsKeyReleased", 1, IsKeyReleased_function);

	world.register_system({
		"KeyboardHandler",
		{
			"UPDATE keyboard SET state = 'pressed' WHERE key = ?",
			"UPDATE keyboard SET state = NULL WHERE state = 'released'",
			"UPDATE keyboard SET state = 'released' WHERE state IN ('pressed', 'hold') AND IsKeyReleased(key)",
			"UPDATE keyboard SET state = 'hold' WHERE state = 'pressed'",
		},
		[](std::vector<ecsql::PreparedSQL>& sqls) {
			auto set_pressed = sqls[0];
			auto reset_released = sqls[1];
			auto update_released = sqls[2];
			auto update_hold = sqls[3];

			// 1. reset all previously released keys
			{
				ZoneScopedN("reset_released");
				reset_released();
			}

			// 2. mark released keys
			{
				ZoneScopedN("update_released");
				update_released();
			}

			// 3. pressed keys that weren't released are now on hold
			{
				ZoneScopedN("update_hold");
				update_hold();
			}

			// 4. add newly pressed keys
			{
				ZoneScopedN("set_pressed");
				while (int key = GetKeyPressed()) {
					set_pressed(key);
				}
			}
		}
	});

	// TODO: handle mouse and gamepad inputs before updating input_action

	world.register_system({
		"UpdateInputActions",
		R"(
			REPLACE INTO input_action(action, state)
			SELECT action, MIN(state) AS state
			FROM input_map
				JOIN keyboard ON input = name
			GROUP BY action
		)"_dedent,
	});

	world.register_system({
		"UpdateInputActions1D",
		R"(
			UPDATE input_action_axis
			SET value = t.value
			FROM (
				SELECT
					input_action_axis.action,
					positive.is_down - negative.is_down AS value
				FROM input_action_axis
					JOIN input_action AS positive ON positive.action = action_positive
					JOIN input_action AS negative ON negative.action = action_negative
			) AS t
			WHERE input_action_axis.action = t.action
		)",
	});
}
