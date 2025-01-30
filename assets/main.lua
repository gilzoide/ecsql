require "components"
require "systems"

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
    SpawnOnKey = {
        scene = "bullet",
        key = "KEY_SPACE",
        cooldown = 0.2,
    },
}
