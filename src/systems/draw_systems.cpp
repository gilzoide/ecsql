#include <string_view>

#include <cdedent.hpp>
#include <raylib.h>
#include <raymath.h>

#include "draw_systems.hpp"
#include "../ecsql/system.hpp"
#include "../flyweights/model_flyweight.hpp"
#include "../flyweights/texture_flyweight.hpp"

void register_draw_systems(ecsql::World& world) {
	world.register_system({
		"ClearScreen",
		R"(
			SELECT ifnull(r, 255), ifnull(g, 255), ifnull(b, 255), ifnull(a, 255)
			FROM screen
		)"_dedent,
		[](auto& get_clear_color) {
			for (ecsql::SQLRow row : get_clear_color()) {
				auto color = row.get<Color>();
				ClearBackground(color);
			}
		},
	});
	world.register_system({
		"DrawTextureRect",
		R"(
			SELECT path, Rectangle.x, Rectangle.y, width, height, Rotation.z, r, g, b, a
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
				DrawTexturePro(tex, source, rectangle, center, rotation, color.value_or(WHITE));
			}
		},
	});
	world.register_system({
		"DrawTexture",
		R"(
			SELECT
				path,
				Position.x, Position.y,
				ifnull(Pivot.x, 0.5), ifnull(Pivot.y, 0.5),
				Rotation.z,
				ifnull(Scale.x, 1), ifnull(Scale.y, 1),
				ifnull(r, 255), ifnull(g, 255), ifnull(b, 255), ifnull(a, 255)
			FROM Texture
				JOIN Position USING(entity_id)
				LEFT JOIN Pivot USING(entity_id)
				LEFT JOIN Rotation USING(entity_id)
				LEFT JOIN Scale USING(entity_id)
				LEFT JOIN Color USING(entity_id)
		)"_dedent,
		[](auto& sql) {
			for (ecsql::SQLRow row : sql()) {
				auto [tex_path, position, normalized_pivot, rotation, scale, color] = row.get<std::string_view, Vector2, Vector2, float, Vector2, Color>();
				auto tex = TextureFlyweight.get(tex_path);
				Rectangle source {
					0, 0,
					(float) tex.value.width, (float) tex.value.height,
				};
				Vector2 pivot { source.width * normalized_pivot.x, source.height * normalized_pivot.y };
				Rectangle dest {
					position.x,
					position.y,
					source.width * scale.x,
					source.height * scale.y,
				};
				DrawTexturePro(tex, source, dest, pivot, rotation, color);
			}
		},
	});
	world.register_system({
		"DrawText",
		R"(
			SELECT
				text, size,
				x, y, width, height,
				ifnull(r, 255), ifnull(g, 255), ifnull(b, 255), ifnull(a, 255)
			FROM Text
				JOIN Rectangle USING(entity_id)
				LEFT JOIN Color USING(entity_id)
		)"_dedent,
		[](auto& sql) {
			for (ecsql::SQLRow row : sql()) {
				auto [text, size, rect, color] = row.get<const char *, int, Rectangle, Color>();
				DrawText(text, rect.x, rect.y, size, color);
			}
		}
	});
	world.register_system({
		"DrawModel",
		{
			R"(
				SELECT
					Position.x, Position.y, Position.z,
					Rotation.x, Rotation.y, Rotation.z,
					fov_y,
					projection = 'orthographic'
				FROM Camera
					LEFT JOIN Position USING(entity_id)
					LEFT JOIN Rotation USING(entity_id)
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
				auto [position, rotation_euler, fov, projection] = row.get<Vector3, Vector3, float, int>();

				Quaternion rotation = QuaternionFromEuler(rotation_euler.x, rotation_euler.y, rotation_euler.z);
				Vector3 target = Vector3RotateByQuaternion(Vector3(0, 0, -1), rotation);
				Vector3 up = Vector3RotateByQuaternion(Vector3(0, 1, 0), rotation);
				Camera3D camera = {
					.position = position,
					.target = target,
					.up = up,
					.fovy = fov,
					.projection = projection,
				};

				BeginMode3D(camera);
				for (ecsql::SQLRow row : get_models()) {
					auto [model_path, position, color] = row.get<std::string_view, Vector3, std::optional<Color>>();
					auto model = ModelFlyweight.get(model_path);
					DrawModel(model, position, 1, color.value_or(WHITE));
				}
				EndMode3D();
			}
		},
	});
}
