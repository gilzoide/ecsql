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
