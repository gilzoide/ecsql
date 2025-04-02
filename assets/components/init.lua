component "BakePosition" {
    "parent_id INTEGER REFERENCES entity(id)",
    "x NOT NULL DEFAULT 0",
    "y NOT NULL DEFAULT 0",
    "z NOT NULL DEFAULT 0",
}

component "DeleteAfter" {
    "seconds NOT NULL",
    "create_uptime",
    additional_schema = [[
        CREATE TRIGGER DeleteAfter_inserted
        AFTER INSERT ON DeleteAfter
        BEGIN
            UPDATE DeleteAfter
            SET create_uptime = uptime
            FROM time
            WHERE entity_id = new.entity_id;
        END;
    ]]
}

component "DestroyOnOutOfScreen" {}

component "ThrustSpeed" {
    "linear NOT NULL DEFAULT 1",
    "angular NOT NULL DEFAULT 1",
}

component "MoveOnArrows" {}

component "MoveVector" {
    "x NOT NULL DEFAULT 0",
    "y NOT NULL DEFAULT 0",
    "z NOT NULL DEFAULT 0",
}

component "ParentOffset" {
    "x NOT NULL DEFAULT 0",
    "y NOT NULL DEFAULT 0",
    "z NOT NULL DEFAULT 0",
    "is_dirty DEFAULT TRUE",
    additional_schema = [[
        CREATE INDEX ParentOffset_is_dirty ON ParentOffset(is_dirty);

        CREATE TRIGGER ParentOffset_Position_updated
        AFTER UPDATE ON Position
        BEGIN
            UPDATE ParentOffset
            SET is_dirty = TRUE
            FROM (
                SELECT id
                FROM entity
                WHERE parent_id = new.entity_id
            ) AS t
            WHERE ParentOffset.entity_id = t.id;
        END;

        CREATE TRIGGER ParentOffset_Rotation_updated
        AFTER UPDATE ON Rotation
        BEGIN
            UPDATE ParentOffset
            SET is_dirty = TRUE
            FROM (
                SELECT id
                FROM entity
                WHERE parent_id = new.entity_id
            ) AS t
            WHERE ParentOffset.entity_id = t.id;
        END;
    ]]
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
