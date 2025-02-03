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

component "ScreenRect" {
    additional_schema = [[
        CREATE TRIGGER ScreenRect_inserted
        AFTER INSERT ON ScreenRect
        BEGIN
            INSERT OR REPLACE INTO Rectangle(entity_id, width, height)
            SELECT new.entity_id, width, height
            FROM screen;
        END;

        CREATE TRIGGER ScreenRect_screen_updated
        AFTER UPDATE OF width, height ON screen
        BEGIN
            UPDATE Rectangle
            SET width = s.width, height = s.height
            FROM (
                SELECT entity_id, width, height
                FROM ScreenRect, screen
            ) AS s
            WHERE Rectangle.entity_id = s.entity_id;
        END;
    ]]
}

component "SpawnOnAction" {
    "scene",
    "cooldown DEFAULT 0",
    "action",
    "last_spawn_time",
}
