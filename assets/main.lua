require "components"
require "systems"

require "input_maps"

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
        cooldown = 0.2,
    },
}
