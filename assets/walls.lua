entity "TopWall" {
    Body = {
        type = "static",
    },
    Shape = {},
    Box = {
        half_width = 400,
        half_height = 10,
    },
    Position = {
        x = 400,
        y = 0,
    },
    Texture = {},
    Color = BLACK,
    Size = {
        width = 800,
        height = 20,
    },
}

entity "LeftWall" {
    Body = {
        type = "static",
    },
    Shape = {},
    Box = {
        half_width = 10,
        half_height = 300,
    },
    Position = {
        x = 0,
        y = 300,
    },
    Texture = {},
    Color = BLACK,
    Size = {
        width = 20,
        height = 600,
    },
}

entity "RightWall" {
    Body = {
        type = "static",
    },
    Shape = {},
    Box = {
        half_width = 10,
        half_height = 300,
    },
    Position = {
        x = 800,
        y = 300,
    },
    Texture = {},
    Color = BLACK,
    Size = {
        width = 20,
        height = 600,
    },
}
