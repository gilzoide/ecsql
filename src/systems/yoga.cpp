#include <string>
#include <string_view>

#include <cdedent.hpp>
#include <flyweight.hpp>
#include <raylib.h>
#include <yoga/Yoga.h>

#include "draw_systems.hpp"
#include "yoga.hpp"
#include "../ecsql/hook_system.hpp"
#include "../ecsql/sql_hook_row.hpp"
#include "../ecsql/system.hpp"

struct YogaNodeContext {
	ecsql::EntityID entity_id;
	float measured_width = -1;
	float measured_height = -1;

	static YogaNodeContext *get(YGNodeConstRef node) {
		return static_cast<YogaNodeContext *>(YGNodeGetContext(node));
	}

	void unset_measured_size(YGNodeRef node) {
		measured_width = measured_height = -1;
		YGNodeSetMeasureFunc(node, nullptr);
	}

	void set_measured_size(YGNodeRef node, float x, float y) {
		measured_width = x;
		measured_height = y;
		YGNodeSetMeasureFunc(node, measure_func);
	}

	static YGSize measure_func(YGNodeConstRef node, float width, YGMeasureMode widthMode, float height, YGMeasureMode heightMode) {
		if (YogaNodeContext *ctx = YogaNodeContext::get(node)) {
			switch (widthMode) {
				case YGMeasureModeUndefined:
					width = ctx->measured_width;
					break;

				case YGMeasureModeAtMost:
					width = std::min(width, ctx->measured_width);
					break;

				default:
					break;
			}
			switch (heightMode) {
				case YGMeasureModeUndefined:
					height = ctx->measured_height;
					break;

				case YGMeasureModeAtMost:
					height = std::min(height, ctx->measured_height);
					break;

				default:
					break;
			}
		}
		return { width, height };
	}
};

flyweight::flyweight_refcounted<ecsql::EntityID, YGNodeRef> YogaNodeFlyweight {
	[](ecsql::EntityID id) {
		YGNodeRef node = YGNodeNew();
		YGNodeSetContext(node, new YogaNodeContext { .entity_id = id });
		return node;
	},
	[](YGNodeRef node) {
		delete YogaNodeContext::get(node);
		YGNodeFree(node);
	},
};

static void YGNodeStyleSetPosition(YGNodeRef node, YGEdge edge, YGValue value) {
	switch (value.unit) {
		case YGUnitUndefined:
		case YGUnitAuto:
			YGNodeStyleSetPosition(node, edge, YGUndefined);
			break;

		case YGUnitPoint:
			YGNodeStyleSetPosition(node, edge, value.value);
			break;

		case YGUnitPercent:
			YGNodeStyleSetPositionPercent(node, edge, value.value);
			break;
	}
}

static void YGNodeStyleSetMargin(YGNodeRef node, YGEdge edge, YGValue value) {
	switch (value.unit) {
		case YGUnitUndefined:
			YGNodeStyleSetMargin(node, edge, YGUndefined);
			break;

		case YGUnitPoint:
			YGNodeStyleSetMargin(node, edge, value.value);
			break;

		case YGUnitPercent:
			YGNodeStyleSetMarginPercent(node, edge, value.value);
			break;

		case YGUnitAuto:
			YGNodeStyleSetMarginAuto(node, edge);
			break;
	}
}

static void YGNodeStyleSetPadding(YGNodeRef node, YGEdge edge, YGValue value) {
	switch (value.unit) {
		case YGUnitUndefined:
		case YGUnitAuto:
			YGNodeStyleSetPadding(node, edge, YGUndefined);
			break;

		case YGUnitPoint:
			YGNodeStyleSetPadding(node, edge, value.value);
			break;

		case YGUnitPercent:
			YGNodeStyleSetPaddingPercent(node, edge, value.value);
			break;
	}
}

static void YGNodeStyleSetFlexBasis(YGNodeRef node, YGValue value) {
	switch (value.unit) {
		case YGUnitUndefined:
			YGNodeStyleSetFlexBasis(node, YGUndefined);
			break;

		case YGUnitPoint:
			YGNodeStyleSetFlexBasis(node, value.value);
			break;

		case YGUnitPercent:
			YGNodeStyleSetFlexBasisPercent(node, value.value);
			break;

		case YGUnitAuto:
			YGNodeStyleSetFlexBasisAuto(node);
			break;
	}
}

static void YGNodeStyleSetWidth(YGNodeRef node, YGValue value) {
	switch (value.unit) {
		case YGUnitUndefined:
			YGNodeStyleSetWidth(node, YGUndefined);
			break;

		case YGUnitPoint:
			YGNodeStyleSetWidth(node, value.value);
			break;

		case YGUnitPercent:
			YGNodeStyleSetWidthPercent(node, value.value);
			break;

		case YGUnitAuto:
			YGNodeStyleSetWidthAuto(node);
			break;
	}
}

static void YGNodeStyleSetHeight(YGNodeRef node, YGValue value) {
	switch (value.unit) {
		case YGUnitUndefined:
			YGNodeStyleSetHeight(node, YGUndefined);
			break;

		case YGUnitPoint:
			YGNodeStyleSetHeight(node, value.value);
			break;

		case YGUnitPercent:
			YGNodeStyleSetHeightPercent(node, value.value);
			break;

		case YGUnitAuto:
			YGNodeStyleSetHeightAuto(node);
			break;
	}
}

static void YGNodeStyleSetMinWidth(YGNodeRef node, YGValue value) {
	switch (value.unit) {
		case YGUnitUndefined:
		case YGUnitAuto:
			YGNodeStyleSetMinWidth(node, YGUndefined);
			break;

		case YGUnitPoint:
			YGNodeStyleSetMinWidth(node, value.value);
			break;

		case YGUnitPercent:
			YGNodeStyleSetMinWidthPercent(node, value.value);
			break;
	}
}

static void YGNodeStyleSetMinHeight(YGNodeRef node, YGValue value) {
	switch (value.unit) {
		case YGUnitUndefined:
		case YGUnitAuto:
			YGNodeStyleSetMinHeight(node, YGUndefined);
			break;

		case YGUnitPoint:
			YGNodeStyleSetMinHeight(node, value.value);
			break;

		case YGUnitPercent:
			YGNodeStyleSetMinHeightPercent(node, value.value);
			break;
	}
}

static void YGNodeStyleSetMaxWidth(YGNodeRef node, YGValue value) {
	switch (value.unit) {
		case YGUnitUndefined:
		case YGUnitAuto:
			YGNodeStyleSetMaxWidth(node, YGUndefined);
			break;

		case YGUnitPoint:
			YGNodeStyleSetMaxWidth(node, value.value);
			break;

		case YGUnitPercent:
			YGNodeStyleSetMaxWidthPercent(node, value.value);
			break;
	}
}

static void YGNodeStyleSetMaxHeight(YGNodeRef node, YGValue value) {
	switch (value.unit) {
		case YGUnitUndefined:
		case YGUnitAuto:
			YGNodeStyleSetMaxHeight(node, YGUndefined);
			break;

		case YGUnitPoint:
			YGNodeStyleSetMaxHeight(node, value.value);
			break;

		case YGUnitPercent:
			YGNodeStyleSetMaxHeightPercent(node, value.value);
			break;
	}
}

static void YGNodeStyleSetGap(YGNodeRef node, YGGutter gutter, YGValue value) {
	switch (value.unit) {
		case YGUnitUndefined:
		case YGUnitAuto:
			YGNodeStyleSetGap(node, gutter, YGUndefined);
			break;

		case YGUnitPoint:
			YGNodeStyleSetGap(node, gutter, value.value);
			break;

		case YGUnitPercent:
			YGNodeStyleSetGapPercent(node, gutter, value.value);
			break;
	}
}

static YGValue to_YGValue(ecsql::SQLBaseRow& row, int index) {
	switch (row.column_type(index)) {
		case SQLITE_NULL:
			return YGValueUndefined;

		case SQLITE_TEXT: {
			std::string_view value = row.column_text(index);
			if (value.ends_with('%')) {
				value.remove_suffix(1);
				return YGValue { std::stof(std::string(value)), YGUnitPercent };
			}
			else if (value == "auto") {
				return YGValueAuto;
			}
			else {
				return YGValue { std::stof(std::string(value)), YGUnitPoint };
			}
		}

		default:
			return YGValue { (float) row.column_double(index), YGUnitPoint };
	}
}


template<typename Fn, typename Value>
static void set_enum(YGNodeRef node, std::string_view column, Fn&& func, Value&& default_value) {
	// base case, set default value if we reached here
	func(node, default_value);
}

template<typename Fn, typename Key, typename Value, typename... Rest>
static void set_enum(YGNodeRef node, std::string_view column, Fn&& func, Value&& default_value, Key&& key, Value&& value, Rest... rest) {
	if (column == key) {
		func(node, value);
	}
	else {
		set_enum(node, column, func, std::forward<Value&&>(default_value), std::forward<Rest>(rest)...);
	}
}

static void setup_yoga_node(YGNodeRef node, ecsql::SQLBaseRow& row) {
	YGNodeRef old_parent = YGNodeGetParent(node);
	if (YGNodeRef *parent = YogaNodeFlyweight.peek(row.get<ecsql::EntityID>(YogaNode_parent_id))) {
		if (old_parent != *parent) {
			if (old_parent) {
				YGNodeRemoveChild(old_parent, node);
			}
			YGNodeInsertChild(*parent, node, YGNodeGetChildCount(*parent));
		}
	}
	else if (old_parent) {
		YGNodeRemoveChild(old_parent, node);
	}

	// Position
	set_enum(node, row.column_text(YogaNode_position),
		YGNodeStyleSetPositionType, YGPositionTypeRelative,
		"relative", YGPositionTypeRelative,
		"absolute", YGPositionTypeAbsolute,
		"static", YGPositionTypeStatic
	);
	YGNodeStyleSetPosition(node, YGEdgeLeft, to_YGValue(row, YogaNode_left));
	YGNodeStyleSetPosition(node, YGEdgeTop, to_YGValue(row, YogaNode_top));
	YGNodeStyleSetPosition(node, YGEdgeRight, to_YGValue(row, YogaNode_right));
	YGNodeStyleSetPosition(node, YGEdgeBottom, to_YGValue(row, YogaNode_bottom));

	// Flex
	set_enum(node, row.column_text(YogaNode_direction),
		YGNodeStyleSetDirection, YGDirectionInherit,
		"inherit", YGDirectionInherit,
		"rtl", YGDirectionRTL,
		"ltr", YGDirectionLTR
	);
	set_enum(node, row.column_text(YogaNode_flex_direction),
		YGNodeStyleSetFlexDirection, YGFlexDirectionColumn,
		"column", YGFlexDirectionColumn,
		"column-reverse", YGFlexDirectionColumnReverse,
		"row", YGFlexDirectionRow,
		"row-reverse", YGFlexDirectionRowReverse
	);
	set_enum(node, row.column_text(YogaNode_flex_wrap),
		YGNodeStyleSetFlexWrap, YGWrapNoWrap,
		"nowrap", YGWrapNoWrap,
		"wrap", YGWrapWrap,
		"wrap-reverse", YGWrapWrapReverse
	);
	YGNodeStyleSetFlexBasis(node, to_YGValue(row, YogaNode_flex_basis));
	YGNodeStyleSetFlexGrow(node, row.get<std::optional<float>>(YogaNode_flex_grow).value_or(YGUndefined));
	YGNodeStyleSetFlexShrink(node, row.get<std::optional<float>>(YogaNode_flex_shrink).value_or(YGUndefined));

	// Alignment
	set_enum(node, row.column_text(YogaNode_justify_content),
		YGNodeStyleSetJustifyContent, YGJustifyFlexStart,
		"flex-start", YGJustifyFlexStart,
		"center", YGJustifyCenter,
		"flex-end", YGJustifyFlexEnd,
		"space-between", YGJustifySpaceBetween,
		"space-around", YGJustifySpaceAround,
		"space-evenly", YGJustifySpaceEvenly
	);
	set_enum(node, row.column_text(YogaNode_align_content),
		YGNodeStyleSetAlignContent, YGAlignFlexStart,
		"auto", YGAlignAuto,
		"flex-start", YGAlignFlexStart,
		"center", YGAlignCenter,
		"flex-end", YGAlignFlexEnd,
		"stretch", YGAlignStretch,
		"baseline", YGAlignBaseline,
		"space-between", YGAlignSpaceBetween,
		"space-around", YGAlignSpaceAround,
		"space-evenly", YGAlignSpaceEvenly
	);
	set_enum(node, row.column_text(YogaNode_align_items),
		YGNodeStyleSetAlignItems, YGAlignStretch,
		"auto", YGAlignAuto,
		"flex-start", YGAlignFlexStart,
		"center", YGAlignCenter,
		"flex-end", YGAlignFlexEnd,
		"stretch", YGAlignStretch,
		"baseline", YGAlignBaseline,
		"space-between", YGAlignSpaceBetween,
		"space-around", YGAlignSpaceAround,
		"space-evenly", YGAlignSpaceEvenly
	);
	set_enum(node, row.column_text(YogaNode_align_self),
		YGNodeStyleSetAlignSelf, YGAlignAuto,
		"auto", YGAlignAuto,
		"flex-start", YGAlignFlexStart,
		"center", YGAlignCenter,
		"flex-end", YGAlignFlexEnd,
		"stretch", YGAlignStretch,
		"baseline", YGAlignBaseline,
		"space-between", YGAlignSpaceBetween,
		"space-around", YGAlignSpaceAround,
		"space-evenly", YGAlignSpaceEvenly
	);

	// Size
	YGNodeStyleSetWidth(node, to_YGValue(row, YogaNode_width));
	YGNodeStyleSetHeight(node, to_YGValue(row, YogaNode_height));

	YGNodeStyleSetMinWidth(node, to_YGValue(row, YogaNode_min_width));
	YGNodeStyleSetMinHeight(node, to_YGValue(row, YogaNode_min_height));

	YGNodeStyleSetMaxWidth(node, to_YGValue(row, YogaNode_max_width));
	YGNodeStyleSetMaxHeight(node, to_YGValue(row, YogaNode_max_height));

	YGNodeStyleSetAspectRatio(node, row.get<std::optional<float>>(YogaNode_aspect_ratio).value_or(YGUndefined));

	YGNodeStyleSetMargin(node, YGEdgeLeft, to_YGValue(row, YogaNode_margin_left));
	YGNodeStyleSetMargin(node, YGEdgeTop, to_YGValue(row, YogaNode_margin_top));
	YGNodeStyleSetMargin(node, YGEdgeRight, to_YGValue(row, YogaNode_margin_right));
	YGNodeStyleSetMargin(node, YGEdgeBottom, to_YGValue(row, YogaNode_margin_bottom));

	YGNodeStyleSetPadding(node, YGEdgeLeft, to_YGValue(row, YogaNode_padding_left));
	YGNodeStyleSetPadding(node, YGEdgeTop, to_YGValue(row, YogaNode_padding_top));
	YGNodeStyleSetPadding(node, YGEdgeRight, to_YGValue(row, YogaNode_padding_right));
	YGNodeStyleSetPadding(node, YGEdgeBottom, to_YGValue(row, YogaNode_padding_bottom));

	YGNodeStyleSetGap(node, YGGutterColumn, to_YGValue(row, YogaNode_column_gap));
	YGNodeStyleSetGap(node, YGGutterRow, to_YGValue(row, YogaNode_row_gap));
}

ecsql::HookSystem YogaNodeHookSystem {
	YogaNode,
	[](ecsql::HookType hook, ecsql::SQLBaseRow& old_row, ecsql::SQLBaseRow& new_row) {
		switch (hook) {
			case ecsql::HookType::OnInsert: {
				YGNodeRef node = YogaNodeFlyweight.get(new_row.get<ecsql::EntityID>());
				setup_yoga_node(node, new_row);
				break;
			}

			case ecsql::HookType::OnUpdate: {
				auto node = YogaNodeFlyweight.get_autorelease(new_row.get<ecsql::EntityID>());
				setup_yoga_node(node, new_row);
				break;
			}

			case ecsql::HookType::OnDelete: {
				YogaNodeFlyweight.release(old_row.get<ecsql::EntityID>());
				break;
			}
		}
	},
};

ecsql::HookSystem TextHookSystem {
	TextComponent,
	[](ecsql::HookType hook, ecsql::SQLBaseRow& old_row, ecsql::SQLBaseRow& new_row) {
		if (hook != ecsql::HookType::OnDelete) {
			return;
		}
		if (YGNodeRef *node_ptr = YogaNodeFlyweight.peek(old_row.get<ecsql::EntityID>())) {
			YGNodeRef node = *node_ptr;
			YogaNodeContext::get(node)->unset_measured_size(node);
		}
	},
};

void recurse_update_rect(YGNodeRef node, ecsql::PreparedSQL& upsert_rectangle) {
	upsert_rectangle(YogaNodeContext::get(node)->entity_id, YGNodeLayoutGetLeft(node), YGNodeLayoutGetTop(node), YGNodeLayoutGetWidth(node), YGNodeLayoutGetHeight(node));
	for (size_t i = 0, count = YGNodeGetChildCount(node); i < count; i++) {
		recurse_update_rect(YGNodeGetChild(node, i), upsert_rectangle);
	}
}

void register_update_yoga(ecsql::World& world) {
	world.register_system({
		"YogaUpdateMeasurements",
		{
			R"(
				SELECT entity_id, text, size
				FROM Text
				JOIN YogaNode USING(entity_id)
				WHERE is_text_dirty
			)"_dedent,
			R"(
				UPDATE YogaNode
				SET is_text_dirty = FALSE
				WHERE is_text_dirty = TRUE
			)"_dedent,
		},
		[](auto& sqls) {
			auto select_text = sqls[0];
			auto reset_dirty_flag = sqls[1];

			for (ecsql::SQLRow row : select_text()) {
				auto [entity_id, text, font_size] = row.get<ecsql::EntityID, const char *, int>();
				float width = MeasureText(text, font_size);
				auto node = YogaNodeFlyweight.get_autorelease(entity_id);
				YogaNodeContext::get(node)->set_measured_size(node, width, font_size);
				YGNodeMarkDirty(node);
			}
			reset_dirty_flag();
		},
	});
	world.register_system({
		"YogaUpdate",
		{
			R"(
				SELECT
					entity_id,
					ifnull(Rectangle.width, screen_size.width), ifnull(Rectangle.height, screen_size.height)
				FROM YogaNode
				LEFT JOIN Rectangle USING(entity_id)
				JOIN screen_size
				WHERE parent_id IS NULL
			)"_dedent,
			RectangleComponent.insert_sql(true),
		},
		[](auto& sqls) {
			auto get_root_yoga_entities = sqls[0];
			auto upsert_rectangle = sqls[1];
			for (ecsql::SQLRow row : get_root_yoga_entities()) {
				auto entity_id = row.get<ecsql::EntityID>(0);
				auto node = YogaNodeFlyweight.get_autorelease(entity_id);
				if (!YGNodeIsDirty(node) && !IsWindowResized()) {
					continue;
				}

				auto [width, height] = row.get<float, float>(1);
				YGNodeCalculateLayout(node, width, height, YGDirectionInherit);
				recurse_update_rect(node, upsert_rectangle);
			}
		},
	});
}
