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

system "DeleteAfterSeconds" {
    [[
        WITH entities_to_delete AS (
            SELECT entity_id
            FROM DeleteAfter
                JOIN time
            WHERE uptime - create_uptime >= seconds
        )
        DELETE FROM entity
        WHERE id IN entities_to_delete
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

system "UpdateUptimeStatistics" {
    [[
        UPDATE save.statistics
        SET total_uptime = total_uptime + time.delta
        FROM time
    ]],
}
