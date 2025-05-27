system "MoveOnArrows" {
    [[
        WITH x AS (
            SELECT value AS x
            FROM input_action_axis
            WHERE action = 'move_x'
        ),
        y AS (
            SELECT value AS y
            FROM input_action_axis
            WHERE action = 'move_y'
        )
        SELECT
            entity_id,
            linear, angular,
            x, -y
        FROM MoveOnArrows
            JOIN ThrustSpeed USING(entity_id)
            JOIN x
            JOIN y
    ]],
    [[
        INSERT INTO Force(entity_id, y, is_local)
        VALUES (?, ?, TRUE)
    ]],
    [[
        INSERT INTO Torque(entity_id, angle)
        VALUES (?, ?)
    ]],
    function(get_movement, add_force, add_torque)
        for row in get_movement() do
            local entity_id, linear, angular, x, y = row:unpack()
            if x ~= 0 then
                add_torque(entity_id, x * angular)
            end
            if y ~= 0 then
                add_force(entity_id, y * linear)
            end
        end
    end,
    use_fixed_delta = true,
}
