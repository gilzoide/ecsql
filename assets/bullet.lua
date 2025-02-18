local BULLET_RADIUS = 13
local get_position = world:prepare_sql([[
SELECT x, y
FROM Position
WHERE entity_id = ?
]], true)

return function(parent_id)
    local x, y = get_position(parent_id):unpack()
    return entity "Bullet" {
        Texture = {
            path = "textures/ballGrey_08.png",
        },
        Position = {
            x = x,
            y = y - 50,
        },
        DestroyOnOutOfScreen = {},
        Body = {
            type = "dynamic",
        },
        Shape = {
            restitution = 1,
        },
        Size = {
            width = BULLET_RADIUS * 2,
            height = BULLET_RADIUS * 2,
        },
        Circle = {
            radius = BULLET_RADIUS,
        },
        LinearVelocity = {
            y = -300,
        },
    }
end
