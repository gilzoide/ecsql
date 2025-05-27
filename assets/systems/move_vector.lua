system "MoveVector" {
    [[
        UPDATE Position
        SET
            x = Position.x + MoveVector.x * time.delta,
            y = Position.y + MoveVector.y * time.delta,
            z = Position.z + MoveVector.z * time.delta
        FROM MoveVector, time
        WHERE Position.entity_id = MoveVector.entity_id
    ]],
}
