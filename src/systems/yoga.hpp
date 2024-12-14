#pragma once

#include "../ecsql/component.hpp"
#include "../ecsql/world.hpp"

enum YogaNodeColumn {
	YogaNode_entity_id,
	YogaNode_parent_id,
	// Position
	YogaNode_position,
	YogaNode_left,
	YogaNode_top,
	YogaNode_right,
	YogaNode_bottom,
	// Flex
	YogaNode_direction,
	YogaNode_flex_direction,
	YogaNode_flex_wrap,
	YogaNode_flex_basis,
	YogaNode_flex_grow,
	YogaNode_flex_shrink,
	// Alignment
	YogaNode_justify_content,
	YogaNode_align_content,
	YogaNode_align_items,
	YogaNode_align_self,
	// Size
	YogaNode_width,
	YogaNode_height,
	YogaNode_min_width,
	YogaNode_min_height,
	YogaNode_max_width,
	YogaNode_max_height,
	YogaNode_aspect_ratio,
	// Margin
	YogaNode_margin_left,
	YogaNode_margin_top,
	YogaNode_margin_right,
	YogaNode_margin_bottom,
	// Padding
	YogaNode_padding_left,
	YogaNode_padding_top,
	YogaNode_padding_right,
	YogaNode_padding_bottom,
	// Gap
	YogaNode_column_gap,
	YogaNode_row_gap,
	// Did text change? Used for setting up measurement function
	is_text_dirty,
};

inline ecsql::Component YogaNode {
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

void register_update_yoga(ecsql::World& world);
