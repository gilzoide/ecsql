local assert, pairs, select, setmetatable, type = assert, pairs, select, setmetatable, type
local table_insert, table_concat, table_unpack = table.insert, table.concat, table.unpack

function system(name, t)
    if t then
        world:register_system(name, t)
    else
        return function(t)
            world:register_system(name, t)
        end
    end
end

function component(name, t)
    if t then
        world:register_component(name, t)
    else
        return function(t)
            world:register_component(name, t)
        end
    end
end

local function create_entity_internal(name, t)
    local entity_id = world:create_entity(name, t.parent_id)
    for component_name, fields in pairs(t) do
        if component_name == "parent_id" then
            goto continue
        end

        local sql = {
            "INSERT INTO ",
            component_name,
            "(entity_id",
        }
        local values = {}
        for field, value in pairs(fields) do
            sql[#sql + 1] = ", "
            sql[#sql + 1] = field
            values[#values + 1] = value
        end

        sql[#sql + 1] = ") VALUES(?"
        for i = 1, #values do
            sql[#sql + 1] = ", ?"
        end
        sql[#sql + 1] = ")"

        world:execute_sql(table_concat(sql), entity_id, table_unpack(values))
        ::continue::
    end
    return entity_id
end

_G.entity = setmetatable({}, {
    __index = {
        delete = function(id)
            return world:delete_entity(id)
        end,
        find = function(name)
            return world:find_entity(name)
        end,
    },
    __call = function(self, name, t)
        if type(name) == "table" then
            return create_entity_internal(t, name)
        elseif t then
            return create_entity_internal(name, t)
        else
            return function(t)
                return create_entity_internal(name, t)
            end
        end
    end,
})

function sql(script, ...)
    if select('#', ...) > 0 or not script:match(";") then
        return world:execute_sql(script, ...)
    else
        world:execute_sql_script(script)
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
			table_insert(notfound, ("\n\tno file %q"):format(filename))
		end
	end
	return nil, table_concat(notfound)
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
