require "components"
require "systems"

require "input_maps"

require "physics"
local create_walls = require "create_walls"

create_walls(800, 600)

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
        angular = 7000000,
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
    Capsule = {
        x1 = 0,
        y1 = 27,
        x2 = 0,
        y2 = -27,
        radius = 10,
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
