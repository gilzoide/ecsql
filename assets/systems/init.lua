system "BakeRelativePosition" {
    [[
        INSERT OR REPLACE INTO Position(entity_id, x, y, z)
        SELECT
            BakePosition.entity_id,
            BakePosition.x + ParentPosition.x,
            BakePosition.y + ParentPosition.y,
            BakePosition.z + ParentPosition.z
        FROM BakePosition
            JOIN Position AS ParentPosition ON BakePosition.parent_id = ParentPosition.entity_id
    ]],
    [[
        DELETE FROM BakePosition
    ]],
}

system "DestroyOnOutOfScreen" {
    [[
        DELETE FROM entity
        WHERE id IN (
            SELECT entity_id
            FROM DestroyOnOutOfScreen
                JOIN Position USING(entity_id)
                JOIN screen
            WHERE position.x NOT BETWEEN 0 AND screen.width
                OR position.y NOT BETWEEN 0 AND screen.height
        )
    ]],
}

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
        SELECT x, -y
        FROM x, y
    ]],
    [[
        UPDATE Position
        SET x = clamp(x + ? * movement.speed, 0, screen_width), y = clamp(y + ? * movement.speed, 0, screen_height)
        FROM (
            SELECT
                entity_id,
                LinearSpeed.speed * time.delta AS speed,
                width AS screen_width, height AS screen_height
            FROM MoveOnArrows
                LEFT JOIN LinearSpeed USING(entity_id)
                JOIN time
                JOIN screen
        ) AS movement
        WHERE Position.entity_id = movement.entity_id
    ]],
    function(get_movement, update_position)
        for row in get_movement() do
            local x, y = row:unpack()
            if x ~= 0 or y ~= 0 then
                local v = Vector2(x, y)
                update_position(v:normalized():unpack())
            end
        end
    end,
}

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

system "SpawnOnAction" {
    [[
        SELECT entity_id, scene
        FROM SpawnOnAction
            JOIN input_action ON SpawnOnAction.action = input_action.action
            JOIN time
        WHERE input_action.is_down
            AND (last_spawn_time IS NULL
                OR last_spawn_time < time.uptime - cooldown)
    ]],
    [[
        UPDATE SpawnOnAction
        SET last_spawn_time = time.uptime
        FROM time
        WHERE entity_id = ?
    ]],
    function(get_spawn_info, update_spawn_time)
        for row in get_spawn_info() do
            local entity_id, scene_path = row:unpack()
            require(scene_path)(entity_id)
            update_spawn_time(entity_id)
        end
    end,
}
