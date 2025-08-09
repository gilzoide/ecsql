local random_position_inside_screen = require "random_position_inside_screen"
local tuning = require "tuning"

return function(position)
    position = position or random_position_inside_screen(tuning.TARGET_RADIUS + tuning.ROOM_DEFAULT_BORDER * 0.5)

    return entity "Target" {
        Sprite = {
            path = "target_red2",
        },
        Size = {
            width = tuning.TARGET_RADIUS * 2,
            height = tuning.TARGET_RADIUS * 2,
        },
        Position = {
            x = position.x,
            y = position.y,
        },
        Body = {
            type = "static",
        },
        Shape = {
            enable_contact_events = true,
        },
        Circle = {
            radius = tuning.TARGET_RADIUS,
        },
        OnBeginContact = {
            callback = "on_target_bullet_contact",
        },
    }
end
