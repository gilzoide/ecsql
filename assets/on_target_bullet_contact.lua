local create_target = require "create_target"

local get_name = world:prepare_sql("SELECT name FROM entity WHERE id = ?", true)

return function(target_id, bullet_id)
    if get_name(bullet_id)[1] ~= "Bullet" then
        return
    end

    entity.delete(target_id)
    create_target()
end
