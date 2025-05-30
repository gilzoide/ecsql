local tuning = require "tuning"

entity "World" {
    World = {
        gravity_x = tuning.GRAVITY_X,
        gravity_y = tuning.GRAVITY_y,
    },
}
