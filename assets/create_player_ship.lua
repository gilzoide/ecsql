local tuning = require "tuning"

return function()
    local player_ship = entity "PlayerShip" {
        Texture = {
            path = "textures/playerShip1_blue.png",
        },
        Position = {
            x = 400,
            y = 500,
        },
        -- Movement
        ThrustSpeed = {
            linear = tuning.PLAYER_SPEED_LINEAR,
            angular = tuning.PLAYER_SPEED_ANGULAR,
        },
        MoveOnArrows = {},
        -- Physics
        Body = {
            type = "dynamic",
            gravity_scale = 0,
            angular_damping = 5,
            linear_damping = 1.5,
        },
        -- Bullet spawner
        SpawnOnAction = {
            scene = "bullet",
            action = "shoot",
            cooldown = tuning.PLAYER_SHOOT_COOLDOWN,
        },
    }
    entity "PlayerShip_Shape1" {
        Shape = {
            body = player_ship,
            density = 0.5,
        },
        Capsule = {
            x1 = 0,
            y1 = 27,
            x2 = 0,
            y2 = -27,
            radius = 10,
        },
    }
    entity "PlayerShip_Shape2" {
        Shape = {
            body = player_ship,
            density = 0.5,
        },
        Capsule = {
            x1 = 38,
            y1 = 5,
            x2 = -38,
            y2 = 5,
            radius = 10,
        },
    }
    return player_ship
end
