local tuning = require "tuning"

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
