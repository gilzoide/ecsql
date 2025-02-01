local key_maps = {
    up = { "KEY_UP", "KEY_W" },
    down = { "KEY_DOWN", "KEY_S" },
    left = { "KEY_LEFT", "KEY_A" },
    right = { "KEY_RIGHT", "KEY_D" },
    shoot = { "KEY_SPACE" },
}
local sql = world:prepare_sql "INSERT INTO input_map(action, input) VALUES(?, ?)"
for action, keys in pairs(key_maps) do
    for _, key in ipairs(keys) do
        sql(action, key)
    end
end
