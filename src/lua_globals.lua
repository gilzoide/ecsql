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
system "name" {
    "SELECT 'sql1'",
    "SELECT 'sql2'",
    function(sql1, sql2)
        -- ...
    end
}
--]]
