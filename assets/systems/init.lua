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
            JOIN Keyboard ON SpawnOnKey.key = Keyboard.name
            JOIN time
        WHERE Keyboard.state IN ('pressed', 'hold')
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
