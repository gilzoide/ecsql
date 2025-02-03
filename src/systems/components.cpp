#include "../ecsql/additional_sql.hpp"
#include "../ecsql/component.hpp"

// Transform components
ecsql::Component PositionComponent {
	"Position",
	{
		"x DEFAULT 0",
		"y DEFAULT 0",
		"z DEFAULT 0",
	}
};

ecsql::Component RotationComponent {
	"Rotation",
	{
		"x DEFAULT 0",
		"y DEFAULT 0",
		"z DEFAULT 0",
	}
};

ecsql::Component ScaleComponent {
	"Scale",
	{
		"x DEFAULT 1",
		"y DEFAULT 1",
		"z DEFAULT 1",
	}
};

ecsql::Component PivotComponent {
	"Pivot",
	{
		"x DEFAULT 0.5",
		"y DEFAULT 0.5",
		"z DEFAULT 0.5",
	}
};

ecsql::Component RectangleComponent {
	"Rectangle",
	{
		"x DEFAULT 0",
		"y DEFAULT 0",
		"width DEFAULT 1",
		"height DEFAULT 1",
	}
};

// Graphics
ecsql::Component ColorComponent {
	"Color",
	{
		"r DEFAULT 255",
		"g DEFAULT 255",
		"b DEFAULT 255",
		"a DEFAULT 255",
	}
};

ecsql::Component TextComponent {
	"Text",
	{
		"text TEXT",
		"size DEFAULT 12",
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
