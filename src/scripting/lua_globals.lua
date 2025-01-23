--[[ Example:
system "name" {
    "SELECT 'sql1'",
    "SELECT 'sql2'",
    function(sql1, sql2)
        -- ...
    end
}
--]]
function system(name, t)
    if t then
        world:register_system(name, t)
    else
        return function(t)
            world:register_system(name, t)
        end
    end
end

--[[ Example:
component "name" {
    "column1",
    "column2 DEFAULT 0",
    "column3 TEXT",
}
--]]
function component(name, t)
    if t then
        world:register_component(name, t)
    else
        return function(t)
            world:register_component(name, t)
        end
    end
end
