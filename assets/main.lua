require "components"
require "systems"

require "input_maps"

require "physics"
require "create_walls"(800, 600)
require "create_player_ship"()

local FullScreenRect = entity "FullScreenRect" {
    YogaNode = {},
    ScreenRect = {},
}
entity "Title" {
    Text = {
        text = "[SPACE] to shoot, [WASD] to move",
        size = 20,
    },
    Color = BLACK,
    YogaNode = {
        parent_id = FullScreenRect,
        top = 20,
        align_self = "center",
    },
}
