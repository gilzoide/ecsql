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
        value = 300,
    },
    MoveOnArrows = {},

    -- Bullet spawner
    SceneSql = {
        path = "bullet.toml",
    },
    SpawnOnKey = {
        scene = "bullet",
        key = "KEY_SPACE",
        cooldown = 0.2,
    },
}
