#include "../ecsql/additional_sql.hpp"
#include "../ecsql/component.hpp"

// Transform components
ecsql::Component PositionComponent {
	"Position",
	{
		"x NOT NULL DEFAULT 0",
		"y NOT NULL DEFAULT 0",
		"z NOT NULL DEFAULT 0",
	}
};
ecsql::Component PreviousPositionComponent {
	"PreviousPosition",
	{
		"x NOT NULL DEFAULT 0",
		"y NOT NULL DEFAULT 0",
		"z NOT NULL DEFAULT 0",
	}
};

ecsql::Component RotationComponent {
	"Rotation",
	{
		"x NOT NULL DEFAULT 0",
		"y NOT NULL DEFAULT 0",
		"z NOT NULL DEFAULT 0",
	}
};
ecsql::Component PreviousRotationComponent {
	"PreviousRotation",
	{
		"x NOT NULL DEFAULT 0",
		"y NOT NULL DEFAULT 0",
		"z NOT NULL DEFAULT 0",
	}
};

ecsql::Component LookAtComponent {
	"LookAt",
	{
		"target_x NOT NULL DEFAULT 0",
		"target_y NOT NULL DEFAULT 0",
		"target_z NOT NULL DEFAULT 0",
		"up_x NOT NULL DEFAULT 0",
		"up_y NOT NULL DEFAULT 1",
		"up_z NOT NULL DEFAULT 0",
	}
};

ecsql::Component ScaleComponent {
	"Scale",
	{
		"x NOT NULL DEFAULT 1",
		"y NOT NULL DEFAULT 1",
		"z NOT NULL DEFAULT 1",
	}
};

ecsql::Component SizeComponent {
	"Size",
	{
		"width DEFAULT 1",
		"height DEFAULT 1",
	}
};

ecsql::Component PivotComponent {
	"Pivot",
	{
		"x NOT NULL DEFAULT 0.5",
		"y NOT NULL DEFAULT 0.5",
		"z NOT NULL DEFAULT 0.5",
	}
};

ecsql::Component RectangleComponent {
	"Rectangle",
	{
		"x NOT NULL DEFAULT 0",
		"y NOT NULL DEFAULT 0",
		"width NOT NULL DEFAULT 1",
		"height NOT NULL DEFAULT 1",
	}
};

// Graphics
ecsql::Component ColorComponent {
	"Color",
	{
		"r NOT NULL DEFAULT 255",
		"g NOT NULL DEFAULT 255",
		"b NOT NULL DEFAULT 255",
		"a NOT NULL DEFAULT 255",
	}
};

ecsql::Component TextComponent {
	"Text",
	{
		"text TEXT",
		"size NOT NULL DEFAULT 12",
	}
};

// Camera
ecsql::Component CameraComponent {
	"Camera",
	{
		 "fov_y NOT NULL DEFAULT 45",
		 "projection DEFAULT 'perspective'",
	}
};
ecsql::Component CameraUpdate {
	"UpdateCamera",
	{
		"mode",
	}
};

// Physics
ecsql::Component LinearVelocityComponent {
	"LinearVelocity",
	{
		"x DEFAULT 0",
		"y DEFAULT 0",
		"z DEFAULT 0",
	}
};

// UI Layout
ecsql::Component YogaNode {
	"YogaNode",
	{
		"parent_id INTEGER REFERENCES YogaNode(entity_id) ON DELETE CASCADE",
		// Position
		"position",
		"left",
		"top",
		"right",
		"bottom",
		// Flex
		"direction",
		"flex_direction",
		"flex_wrap",
		"flex_basis",
		"flex_grow",
		"flex_shrink",
		// Alignment
		"justify_content",
		"align_content",
		"align_items",
		"align_self",
		// Size
		"width",
		"height",
		"min_width",
		"min_height",
		"max_width",
		"max_height",
		"aspect_ratio",
		// Margin
		"margin_left",
		"margin_top",
		"margin_right",
		"margin_bottom",
		// Padding
		"padding_left",
		"padding_top",
		"padding_right",
		"padding_bottom",
		// Gap
		"column_gap",
		"row_gap",
		// Did text change? Used for setting up measurement function
		"is_text_dirty",
	},
	"CREATE INDEX YogaNode_is_text_dirty ON YogaNode(is_text_dirty);"
};

ecsql::AdditionalSQL YogaNodeTriggers(R"(
	CREATE TRIGGER YogaNode_OnInsert
	AFTER INSERT ON YogaNode
	BEGIN
		UPDATE YogaNode
		SET is_text_dirty = exists(SELECT entity_id FROM Text WHERE entity_id = new.entity_id)
		WHERE entity_id = new.entity_id;
	END;

	CREATE TRIGGER YogaNode_OnInsertText
	AFTER INSERT ON Text
	BEGIN
		UPDATE YogaNode
		SET is_text_dirty = TRUE
		WHERE entity_id = new.entity_id;
	END;

	CREATE TRIGGER YogaNode_OnUpdateText
	AFTER UPDATE ON Text
	BEGIN
		UPDATE YogaNode
		SET is_text_dirty = TRUE
		WHERE entity_id = new.entity_id;
	END;

	CREATE TRIGGER YogaNode_OnDeleteText
	AFTER DELETE ON Text
	BEGIN
		UPDATE YogaNode
		SET is_text_dirty = FALSE
		WHERE entity_id = old.entity_id;
	END;
)");
