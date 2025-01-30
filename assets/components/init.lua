component "BakePosition" {
    "parent_id INTEGER REFERENCES entity(id)",
    "x DEFAULT 0",
    "y DEFAULT 0",
    "z DEFAULT 0",
}

component "DestroyOnOutOfScreen" {}

component "LinearSpeed" {
    "speed DEFAULT 1"
}

component "MoveOnArrows" {}

component "MoveVector" {
    "x DEFAULT 0",
    "y DEFAULT 0",
}

component "SpawnOnKey" {
    "scene",
    "cooldown DEFAULT 0",
    "key",
    "last_spawn_time",
}
