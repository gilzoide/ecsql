local tuning = require "tuning"

require "save_data"

require "components"
require "systems"

require "input_maps"

require "physics"
require "create_walls"(tuning.ROOM_WIDTH, tuning.ROOM_HEIGHT)
require "create_player_ship"()

local FullScreenRect = entity "FullScreenRect" {
    YogaNode = {},
    Rectangle = {
        x = 0,
        y = 0,
        width = tuning.ROOM_WIDTH,
        height = tuning.ROOM_HEIGHT,
    }
}
entity "Title" {
    Text = {
        text = "[SPACE] to shoot, [WASD] to move",
        size = 20,
    },
    Color = BLACK,
    YogaNode = {
        parent_id = FullScreenRect,
        top = 30,
        align_self = "center",
    },
}
entity "TextVSpacing" {
    YogaNode = {
        parent_id = FullScreenRect,
        flex_grow = 1,
    },
}
entity "UptimeText" {
    Text = {
        text = "oi",
        size = 20,
    },
    Color = BLACK,
    YogaNode = {
        parent_id = FullScreenRect,
        margin_right = 16,
        margin_bottom = 16,
        align_self = "flex-end",
    },
    UptimeText = {},
}
