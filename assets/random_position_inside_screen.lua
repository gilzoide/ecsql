local tuning = require "tuning"

return function(radius)
    radius = radius or 0
    local position = Vector2(
        math.random(radius, tuning.ROOM_WIDTH - radius),
        math.random(radius, tuning.ROOM_HEIGHT - radius)
    )
    return position
end
