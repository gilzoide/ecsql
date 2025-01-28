system "MoveVector" {
    [[
        UPDATE Position
        SET x = Position.x + movement.x, y = Position.y + movement.y
        FROM (
            SELECT entity_id, ifnull(MoveVector.x, 0) * time.delta AS x, ifnull(MoveVector.y, 0) * time.delta AS y
            FROM MoveVector, time
        ) AS movement
        WHERE Position.entity_id = movement.entity_id
    ]],
    function(move_vector)
        move_vector()
    end,
}
