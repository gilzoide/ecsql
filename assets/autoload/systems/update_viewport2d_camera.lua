system "UpdateViewport2DCamera" {
    [[
        SELECT
            entity_id,
            V.x, V.y, V.width, V.height,
            R.x, R.y, R.width, R.height
        FROM Viewport2D AS V
            JOIN Rectangle AS R USING(entity_id)
        WHERE is_dirty
    ]],
    [[
        REPLACE INTO Camera2D(entity_id, offset_x, offset_y, target_x, target_y, zoom)
        VALUES(?, ?, ?, ?, ?, ?)
    ]],
    [[
        UPDATE Viewport2D
        SET is_dirty = FALSE
        WHERE is_dirty = TRUE
    ]],
    function(get_dirty_viewports, upsert_camera, reset_dirty)
        for row in get_dirty_viewports() do
            local entity_id, V_x, V_y, V_width, V_height, R_x, R_y, R_width, R_height = row:unpack()
            local V_pos = Vector2(V_x, V_y)
            local V_size = Vector2(V_width, V_height)
            local V_center = V_pos + (V_size / 2)
            local R_pos = Vector2(R_x, R_y)
            local R_size = Vector2(R_width, R_height)
            local R_center = R_pos + (R_size / 2)

            local target = V_center
            local offset = R_center
            local zoom
            -- Rect is more wide than Viewport: fit vertically
            if V_size:aspect() < R_size:aspect() then
                zoom = R_size.y / V_size.y
            else
                zoom = R_size.x / V_size.x
            end

            upsert_camera(entity_id, offset.x, offset.y, target.x, target.y, zoom)
        end
        reset_dirty()
    end
}
