return function(width, height, border)
    local half_width = width / 2
    local half_height = height / 2
    border = border or 20
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
        Texture = {},
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
        Texture = {},
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
        Texture = {},
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
        Texture = {},
        Color = BLACK,
        Size = {
            width = border,
            height = height + border,
        },
    }
end
