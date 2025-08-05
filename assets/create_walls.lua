local tuning = require "tuning"

return function(width, height, border)
    width = width or tuning.ROOM_WIDTH
    height = height or tuning.ROOM_HEIGHT
    border = border or tuning.ROOM_DEFAULT_BORDER

    local half_width = width / 2
    local half_height = height / 2
    local half_border = border / 2

    entity "TopWall" {
        Body = {
            type = "static",
        },
        Shape = {},
        Box = {
            half_width = half_width,
            half_height = half_border,
        },
        Position = {
            x = half_width,
            y = 0,
        },
        Sprite = {},
        Color = BLACK,
        Size = {
            width = width + border,
            height = border,
        },
    }

    entity "BottomWall" {
        Body = {
            type = "static",
        },
        Shape = {},
        Box = {
            half_width = half_width,
            half_height = half_border,
        },
        Position = {
            x = half_width,
            y = height,
        },
        Sprite = {},
        Color = BLACK,
        Size = {
            width = width + border,
            height = border,
        },
    }

    entity "LeftWall" {
        Body = {
            type = "static",
        },
        Shape = {},
        Box = {
            half_width = half_border,
            half_height = half_height,
        },
        Position = {
            x = 0,
            y = half_height,
        },
        Sprite = {},
        Color = BLACK,
        Size = {
            width = border,
            height = height + border,
        },
    }

    entity "RightWall" {
        Body = {
            type = "static",
        },
        Shape = {},
        Box = {
            half_width = half_border,
            half_height = half_height,
        },
        Position = {
            x = width,
            y = half_height,
        },
        Sprite = {},
        Color = BLACK,
        Size = {
            width = border,
            height = height + border,
        },
    }
end
