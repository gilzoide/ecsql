#include <string_view>

#include <cdedent.hpp>
#include <raylib.h>
#include <raymath.h>

#include "draw_systems.hpp"
#include "../ecsql/system.hpp"
#include "../flyweights/model_flyweight.hpp"
#include "../flyweights/texture_flyweight.hpp"

#define DEFAULT_CLEAR_COLOR WHITE
#define DEFAULT_TEXT_COLOR BLACK

void register_draw_systems(ecsql::World& world) {
	world.register_system({
		"ClearScreen",
		R"(
			SELECT r, g, b, a
			FROM screen
		)"_dedent,
		[](auto& get_clear_color) {
			for (ecsql::SQLRow row : get_clear_color()) {
				auto color = row.get<std::optional<Color>>();
				{
					ZoneScopedN("ClearBackground");
					ClearBackground(color.value_or(DEFAULT_CLEAR_COLOR));
				}
			}
		},
	});
	world.register_system({
		"DrawTextureRect",
		R"(
			SELECT
				path,
				Rectangle.x, Rectangle.y, width, height,
				Rotation.z,
				r, g, b, a
			FROM Texture
				JOIN Rectangle USING(entity_id)
				LEFT JOIN Rotation USING(entity_id)
				LEFT JOIN Color USING(entity_id)
		)"_dedent,
		[](auto& sql) {
			for (ecsql::SQLRow row : sql()) {
				auto [tex_path, rectangle, rotation, color] = row.get<std::string_view, Rectangle, float, std::optional<Color>>();
				auto tex = TextureFlyweight.get(tex_path);
				Rectangle source {
					0, 0,
					(float) tex.value.width, (float) tex.value.height,
				};
				Vector2 center { rectangle.width * 0.5f, rectangle.height * 0.5f };
				rectangle.x += center.x;
				rectangle.y += center.y;
				{
					ZoneScopedN("DrawTexturePro");
					DrawTexturePro(tex, source, rectangle, center, rotation, color.value_or(WHITE));
				}
			}
		},
	});
	world.register_system({
		"DrawTexture",
		R"(
			SELECT
				path,
				Position.x, Position.y,
				Pivot.x, Pivot.y,
				Rotation.z,
				Scale.x, Scale.y,
				r, g, b, a
			FROM Texture
				JOIN Position USING(entity_id)
				LEFT JOIN Pivot USING(entity_id)
				LEFT JOIN Rotation USING(entity_id)
				LEFT JOIN Scale USING(entity_id)
				LEFT JOIN Color USING(entity_id)
		)"_dedent,
		[](auto& sql) {
			for (ecsql::SQLRow row : sql()) {
				auto [tex_path, position, normalized_pivot, rotation, scale, color] = row.get<std::string_view, Vector2, std::optional<Vector2>, float, std::optional<Vector2>, std::optional<Color>>();
				if (!normalized_pivot) normalized_pivot.emplace(0.5, 0.5);
				if (!scale) scale.emplace(1, 1);

				auto tex = TextureFlyweight.get(tex_path);
				Rectangle source {
					0, 0,
					(float) tex.value.width, (float) tex.value.height,
				};
				Rectangle dest {
					position.x,
					position.y,
					source.width * scale->x,
					source.height * scale->y,
				};
				Vector2 pivot { dest.width * normalized_pivot->x, dest.height * normalized_pivot->y };
				{
					ZoneScopedN("DrawTexturePro");
					DrawTexturePro(tex, source, dest, pivot, rotation, color.value_or(WHITE));
				}
			}
		},
	});
	world.register_system({
		"DrawText",
		R"(
			SELECT
				text, size,
				x, y, width, height,
				r, g, b, a
			FROM Text
				JOIN Rectangle USING(entity_id)
				LEFT JOIN Color USING(entity_id)
		)"_dedent,
		[](auto& sql) {
			for (ecsql::SQLRow row : sql()) {
				auto [text, size, rect, color] = row.get<const char *, int, Rectangle, std::optional<Color>>();
				{
					ZoneScopedN("DrawText");
					DrawText(text, rect.x, rect.y, size, color.value_or(DEFAULT_TEXT_COLOR));
				}
			}
		}
	});
	world.register_system({
		"DrawModel",
		{
			R"(
				SELECT
					Position.x, Position.y, Position.z,
					LookAt.target_x, LookAt.target_y, LookAt.target_z,
					LookAt.up_x, coalesce(LookAt.up_y, 1), LookAt.up_z,
					fov_y,
					projection = 'orthographic'
				FROM Camera
					LEFT JOIN Position USING(entity_id)
					LEFT JOIN LookAt USING(entity_id)
			)",
			R"(
				SELECT
					path,
					x, y, z,
					r, g, b, a
				FROM Model
					JOIN Position USING(entity_id)
					LEFT JOIN Color USING(entity_id)
			)"_dedent,
		},
		[](auto& sqls) {
			auto get_cameras = sqls[0];
			auto get_models = sqls[1];

			for (ecsql::SQLRow row : get_cameras()) {
				auto [position, target, up, fov, projection] = row.get<Vector3, Vector3, Vector3, float, int>();
				Camera3D camera = {
					.position = position,
					.target = target,
					.up = up,
					.fovy = fov,
					.projection = projection,
				};

				{
					ZoneScopedN("BeginMode3D");
					BeginMode3D(camera);
				}
				for (ecsql::SQLRow row : get_models()) {
					auto [model_path, position, color] = row.get<std::string_view, Vector3, std::optional<Color>>();
					auto model = ModelFlyweight.get(model_path);
					{
						ZoneScopedN("DrawModel");
						DrawModel(model, position, 1, color.value_or(WHITE));
					}
				}
				{
					ZoneScopedN("EndMode3D");
					EndMode3D();
				}
			}
		},
	});
	world.register_system({
		"UpdateCamera",
		{
			R"(
				SELECT
					entity_id,
					mode,
					Position.x, Position.y, Position.z,
					LookAt.target_x, LookAt.target_y, LookAt.target_z,
					LookAt.up_x, coalesce(LookAt.up_y, 1), LookAt.up_z,
					fov_y,
					projection = 'orthographic'
				FROM UpdateCamera
					JOIN Camera USING(entity_id)
					LEFT JOIN Position USING(entity_id)
					LEFT JOIN LookAt USING(entity_id)
			)"_dedent,
			R"(
				REPLACE INTO Position
				VALUES(?, ?, ?, ?)
			)",
			R"(
				REPLACE INTO LookAt
				VALUES(?, ?, ?, ?, ?, ?, ?)
			)",
		},
		[](auto& sqls) {
			auto get_cameras = sqls[0];
			auto set_position = sqls[1];
			auto set_look_at = sqls[2];

			for (ecsql::SQLRow row : get_cameras()) {
				auto [entity_id, update_mode, position, target, up, fov, projection] = row.get<ecsql::EntityID, std::string_view, Vector3, Vector3, Vector3, float, int>();
				Camera3D camera = {
					.position = position,
					.target = target,
					.up = up,
					.fovy = fov,
					.projection = projection,
				};
				if (update_mode == "free") {
					UpdateCamera(&camera, CAMERA_FREE);
				}
				else if (update_mode == "orbital") {
					UpdateCamera(&camera, CAMERA_ORBITAL);
				}
				else if (update_mode == "first_person") {
					UpdateCamera(&camera, CAMERA_FIRST_PERSON);
				}
				else if (update_mode == "third_person") {
					UpdateCamera(&camera, CAMERA_THIRD_PERSON);
				}
				else {
					continue;
				}

				Vector3 final_position = camera.position;
				Vector3 final_target = camera.target;
				Vector3 final_up = camera.up;

				if (final_position != position) {
					set_position(entity_id, final_position);
				}
				if (final_target != target || final_up != up) {
					set_look_at(entity_id, final_target, final_up);
				}
			}
		},
	});
}
