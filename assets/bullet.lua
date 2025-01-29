return function(parent_id)
    return entity "Bullet" {
        Texture = {
            path = "textures/laserBlue01.png",
        },
        MoveVector = {
            x = 0,
            y = -500,
        },
        BakePosition = {
            parent_id = parent_id,
            y = -50,
        },
        DestroyOnOutOfScreen = {},
    }
end
