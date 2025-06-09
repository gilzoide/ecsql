local assert, pairs, select, type = assert, pairs, select, type
local table_insert, table_concat, table_unpack = table.insert, table.concat, table.unpack

--- @param name string
--- @param t table|nil
--- @return nil|function
function system(name, t)
    if t then
        world:register_system(name, t, t.use_fixed_delta)
    else
        return function(t)
            world:register_system(name, t, t.use_fixed_delta)
        end
    end
end

--- @param name string
--- @param t table|nil
--- @return nil|function
function component(name, t)
    if t then
        world:register_component(name, t)
    else
        return function(t)
            world:register_component(name, t)
        end
    end
end

--- @param component_name string
--- @param f function
function hook_system(component_name, f)
    world:register_hook_system(component_name, f)
end

local function create_component_internal(component_name, entity_id, fields)
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
end

local function create_entity_internal(name, t)
    local entity_id = world:create_entity(name, t.parent_id)
    for component_name, fields in pairs(t) do
        if component_name == "parent_id" then
            goto continue
        end

        if #fields > 0 then
            for i = 1, #fields do
                create_component_internal(component_name, entity_id, fields[i])
            end
        else
            create_component_internal(component_name, entity_id, fields)
        end
        ::continue::
    end
    return entity_id
end

--- @param name string|table
--- @param t table|nil
--- @return integer|function
function entity(name, t)
    if type(name) == "table" then
        return create_entity_internal(t, name)
    elseif t then
        return create_entity_internal(name, t)
    else
        return function(t)
            return create_entity_internal(name, t)
        end
    end
end

--- @param script string
--- @return ExecutedSQL|nil
function sql(script, ...)
    if select('#', ...) > 0 or not script:match(";") then
        return world:execute_sql(script, ...)
    else
        world:execute_sql_script(script)
    end
end

--- @param f function
function inside_transaction(f)
    sql("BEGIN")
    local success = pcall(f)
    if success then
        sql("COMMIT")
    else
        sql("ROLLBACK")
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
