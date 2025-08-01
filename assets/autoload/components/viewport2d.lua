component "Viewport2D" {
    "x NOT NULL DEFAULT 0",
    "y NOT NULL DEFAULT 0",
    "width NOT NULL DEFAULT 1024",
    "height NOT NULL DEFAULT 768",
    "is_dirty DEFAULT TRUE",
    additional_schema = [[
        CREATE INDEX Viewport2D_is_dirty ON Viewport2D(is_dirty);

        CREATE TRIGGER Viewport2D_OnUpdateRect
        AFTER UPDATE ON Rectangle
        BEGIN
            UPDATE Viewport2D
            SET is_dirty = TRUE
            WHERE entity_id = new.entity_id;
        END;
    ]]
}
