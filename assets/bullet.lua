local tuning = require "tuning"

local get_position = world:prepare_sql([[
SELECT
    Position.x, Position.y,
    coalesce(Rotation.z, 0)
FROM Position
    LEFT JOIN Rotation USING(entity_id)
WHERE entity_id = ?
]], true)

return function(parent_id)
    local x, y, rotation = get_position(parent_id):unpack()
    local offset = Vector2(0, -50):rotated(rotation * DEG2RAD)
    local impulse = Vector2(0, -100000):rotated(rotation * DEG2RAD)
    return entity "Bullet" {
        Texture = {
            path = "textures/ballGrey_08.png",
        },
        Position = {
            x = x + offset.x,
            y = y + offset.y,
        },
        Rotation = {
            z = rotation,
        },
        DestroyOnOutOfScreen = {},
        Body = {
            type = "dynamic",
        },
        Shape = {
            restitution = 1,
        },
        Size = {
            width = tuning.BULLET_RADIUS * 2,
            height = tuning.BULLET_RADIUS * 2,
        },
        Circle = {
            radius = tuning.BULLET_RADIUS,
        },
        LinearImpulse = {
            x = impulse.x,
            y = impulse.y,
        },
        DeleteAfter = {
            seconds = tuning.BULLET_LIFETIME,
        },
    }
end
