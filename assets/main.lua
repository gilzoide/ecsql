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
    ThrustSpeed = {
        linear = 700000,
        angular = 10000000,
    },
    MoveOnArrows = {},

    -- Physics
    Body = {
        type = "dynamic",
        gravity_scale = 0,
        angular_damping = 5,
        linear_damping = 1.5,
    },
    Shape = {
    },
    Box = {
        half_width = 20,
        half_height = 35,
    },

    -- Bullet spawner
    SpawnOnAction = {
        scene = "bullet",
        action = "shoot",
        cooldown = 0.6,
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
