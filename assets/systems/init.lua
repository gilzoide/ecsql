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
    function(bake_position, delete_bake_position)
        bake_position()
        delete_bake_position()
    end,
}

system "DestroyOnOutOfScreen" {
    [[
        DELETE FROM entity
        WHERE id IN (
            SELECT entity_id
            FROM DestroyOnOutOfScreen
                JOIN Position USING(entity_id)
                JOIN screen_size
            WHERE position.x NOT BETWEEN 0 AND screen_size.width
                OR position.y NOT BETWEEN 0 AND screen_size.height
        )
    ]],
    function(delete_out_of_screen)
        delete_out_of_screen()
    end,
}

system "MoveOnArrows" {
    [[
        WITH up AS (
            SELECT MAX(is_down) AS up
            FROM keyboard
            WHERE name IN ('KEY_UP', 'KEY_W')
        ),
        down AS (
            SELECT MAX(is_down) AS down
            FROM keyboard
            WHERE name IN ('KEY_DOWN', 'KEY_S')
        ),
        left AS (
            SELECT MAX(is_down) AS left
            FROM keyboard
            WHERE name IN ('KEY_LEFT', 'KEY_A')
        ),
        right AS (
            SELECT MAX(is_down) AS right
            FROM keyboard
            WHERE name IN ('KEY_RIGHT', 'KEY_D')
        )
        SELECT ifnull(right, 0) - ifnull(left, 0) AS x, ifnull(down, 0) - ifnull(up, 0) AS y
        FROM up, down, left, right
    ]],
    [[
        UPDATE Position
        SET x = clamp(x + ? * movement.speed, 0, screen_width), y = clamp(y + ? * movement.speed, 0, screen_height)
        FROM (
            SELECT
                entity_id,
                ifnull(LinearSpeed.speed, 1) * time.delta AS speed,
                width AS screen_width, height AS screen_height
            FROM MoveOnArrows
                LEFT JOIN LinearSpeed USING(entity_id)
                JOIN time
                JOIN screen_size
        ) AS movement
        WHERE Position.entity_id = movement.entity_id
    ]],
    function(get_movement, update_position)
        for row in get_movement() do
            local x, y = row:unpack()
            update_position(Vector2(x, y):normalized():unpack())
        end
    end,
}

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

system "SpawnOnKey" {
    [[
        SELECT entity_id, scene
        FROM SpawnOnKey
            JOIN keyboard ON SpawnOnKey.key = keyboard.name
            JOIN time
        WHERE keyboard.is_down
            AND (last_spawn_time IS NULL
                OR last_spawn_time < time.uptime - cooldown)
    ]],
    [[
        UPDATE SpawnOnKey
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
