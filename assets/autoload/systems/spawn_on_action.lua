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
