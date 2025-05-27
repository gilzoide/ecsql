system "UpdateParentOffset" {
    [[
        SELECT
            ParentOffset.entity_id AS entity_id,
            ParentOffset.x AS local_x,
            ParentOffset.y AS local_y,
            parent_position.x AS parent_x,
            parent_position.y AS parent_y,
            coalesce(parent_rotation.z, 0) AS rotation
        FROM ParentOffset
            JOIN entity ON ParentOffset.entity_id = entity.id
            JOIN Position AS parent_position ON parent_position.entity_id = parent_id
            LEFT JOIN Rotation AS parent_rotation ON parent_rotation.entity_id = parent_id
        WHERE is_dirty
    ]],
    [[
        REPLACE INTO Position(entity_id, x, y)
        VALUES(?, ?, ?)
    ]],
    [[
        REPLACE INTO Rotation(entity_id, z)
        VALUES(?, ?)
    ]],
    function(select_values, update_position, update_rotation)
        for row in select_values() do
            local entity_id, local_x, local_y, parent_x, parent_y, rotation = row:unpack()
            local local_position = Vector2(local_x, local_y):rotated(rotation * DEG2RAD)
            local global_position = Vector2(parent_x, parent_y) + local_position
            update_position(entity_id, global_position:unpack())
            update_rotation(entity_id, rotation)
        end
    end,
}
