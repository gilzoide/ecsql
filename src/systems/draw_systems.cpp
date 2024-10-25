#include <string_view>

#include <cdedent.hpp>
#include <raylib.h>

#include "draw_systems.hpp"
#include "../ecsql/system.hpp"
#include "../flyweights/model_flyweight.hpp"
#include "../flyweights/texture_flyweight.hpp"

void register_draw_systems(ecsql::Ecsql& world) {
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
				ifnull(r, 255), ifnull(g, 255), ifnull(b, 255), ifnull(a, 255)
			FROM Texture
			JOIN Position USING(entity_id)
			LEFT JOIN Pivot USING(entity_id)
			LEFT JOIN Rotation USING(entity_id)
			LEFT JOIN Color USING(entity_id)
		)"_dedent,
		[](auto& sql) {
			for (ecsql::SQLRow row : sql()) {
				auto [tex_path, position, normalized_pivot, rotation, color] = row.get<std::string_view, Vector2, Vector2, float, Color>();
				auto tex = TextureFlyweight.get(tex_path);
				Rectangle source {
					0, 0,
					(float) tex.value.width, (float) tex.value.height,
				};
				Vector2 pivot { source.width * normalized_pivot.x, source.height * normalized_pivot.y };
				Rectangle dest {
					position.x,
					position.y,
					source.width,
					source.height,
				};
				DrawTexturePro(tex, source, dest, pivot, rotation, color);
			}
		},
	});
	world.register_system({
		"DrawModel",
		R"(
			SELECT path, Position.x, Position.y, Position.z, r, g, b, a
			FROM Model
				JOIN Position USING(entity_id)
				LEFT JOIN Color USING(entity_id)
		)"_dedent,
		[](auto& sql) {
			Camera3D camera {
				{ 10, 10, 10 },
				{ 0, 0, 0 },
				{ 0, 1, 0 },
				45,
				CAMERA_PERSPECTIVE,
			};
			BeginMode3D(camera);
			for (ecsql::SQLRow row : sql()) {
				auto [model_path, position, color] = row.get<std::string_view, Vector3, std::optional<Color>>();
				auto model = ModelFlyweight.get(model_path);
				DrawModel(model, position, 1, color.value_or(WHITE));
			}
			EndMode3D();
		},
	});
}
