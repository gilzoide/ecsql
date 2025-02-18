require "components"
require "systems"

require "input_maps"

require "physics"
require "walls"

entity "PlayerShip" {
    Texture = {
        path = "textures/playerShip1_blue.png",
    },
    Position = {
        x = 400,
        y = 500,
    },
    LinearSpeed = {
        speed = 300,
    },
    MoveOnArrows = {},

    -- Bullet spawner
    SpawnOnAction = {
        scene = "bullet",
        action = "shoot",
        cooldown = 1,
    },
}

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
