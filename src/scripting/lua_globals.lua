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

    -- Optional arguments
    additional_schema = "CREATE INDEX ...",
    allow_duplicates = true,
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

--[[ Example:
entity "name" {
    component1 = {
        col1 = "value",
        col2 = 42,
        col3 = true,
    },
    component2 = {},
}

entity {
    -- ...
}
--]]
function entity(name, t)
    if type(name) == "table" then
        world:create_entity(t, name)
    elseif t then
        world:create_entity(name, t)
    else
        return function(t)
            world:create_entity(name, t)
        end
    end
end
