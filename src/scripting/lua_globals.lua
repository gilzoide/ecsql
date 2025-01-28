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

-- Patch package Lua loader using PhysFS
local execdir_repl = ecsql.file_base_dir()
local function searchpath(name, path, sep, rep)
	sep = sep or '.'
	rep = rep or '/'
	if sep ~= '' then
		name = name:replace(sep, rep)
	end
	local notfound = {}
	for template in path:gmatch('[^;]+') do
		local filename = template:replace('?', name):replace('!', execdir_repl)
        if ecsql.file_exists(filename) then
			return filename
		else
			table.insert(notfound, ("\n\tno file %q"):format(filename))
		end
	end
	return nil, table.concat(notfound)
end
package.searchpath = searchpath

local function lua_searcher(name)
	local filename, err = searchpath(name, package.path)
	if not filename then
		return err
	end
	return assert(ecsql.loadfile(filename))
end

local searchers = package.searchers or package.loaders
searchers[2] = lua_searcher

package.path = "?.lua;?/init.lua;!/?.lua;!/?/init.lua"
