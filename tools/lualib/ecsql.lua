--- @type boolean|nil
DEBUG = nil

--- @param s string
--- @param pattern string
--- @param replacement string
--- @return string
function string.replace(s, pattern, replacement) return "" end

--- @class entity
--- @overload fun(name: string|table, t: nil|table): nil
entity = {
    --- @param id integer|string
    --- @return integer
    delete = function(id) return 0 end,

    --- @param name string
    --- @return integer|nil
    find = function(name) return 0 end,
}

function component(name, t) end
function system(name, t) end

--- @param script string
function sql(script, ...) end

--- @param f function
function inside_transaction(f) end

ecsql = {
    --- @param path string
    --- @return string
    file_exists = function(path) return "" end,

    --- @return string
    file_base_dir = function() return "" end,

    --- @param filename string
    --- @return function|nil
    --- @return nil|string
    loadfile = function(filename) return nil, "error" end,
}

-- Usertypes

--- @class World
World = {
    --- @param self World
    --- @param name string
    --- @param t table
    --- @param use_fixed_delta boolean
    register_system = function(self, name, t, use_fixed_delta) end,

    --- @param self World
    --- @param name string
    --- @param t table
    register_component = function(self, name, t) end,

    --- @param self World
    --- @param name string|nil
    --- @param parent_id integer|nil
    --- @return integer
    create_entity = function(self, name, parent_id) return 0 end,

    --- @param self World
    --- @param id_or_name integer|string
    delete_entity = function(self, id_or_name) return 0 end,

    --- @param self World
    --- @param name string
    --- @return integer|nil
    find_entity = function(self, name) return nil end,

    --- @param self World
    --- @param sql string
    --- @param is_persistent boolean|nil
    --- @return PreparedSQL
    prepare_sql = function(self, sql, is_persistent) return {} end,

    --- @param self World
    --- @param sql string
    --- @return ExecutedSQL
    execute_sql = function(self, sql, ...) return {} end,

    --- @param self World
    --- @param sql string
    execute_sql_script = function(self, sql) end,

    --- @param self World
    --- @param filename string
    --- @param db_name string|nil
    backup_into = function(self, filename, db_name) end,
}

--- @class PreparedSQL
--- @overload fun(...): ExecutedSQL
PreparedSQL = {
}

--- @class ExecutedSQL
--- @operator len: integer
--- @overload fun(...): SQLRowIterator|nil
ExecutedSQL = {
    --- @param self ExecutedSQL
    --- @return ...
    unpack = function(self) return table.unpack(self) end
}

--- @class SQLRowIterator
--- @operator len: integer
SQLRowIterator = {
    --- @param self SQLRowIterator
    --- @return ...
    unpack = function(self) return table.unpack(self) end
}

--- @class Vector2
--- @operator add(Vector2):Vector2
--- @overload fun(x: number, y: number): Vector2
Vector2 = {
    x = 0.0,
    y = 0.0,

    --- @param self Vector2
    --- @return Vector2
    normalized = function(self) return self end,

    --- @param self Vector2
    --- @param angle number
    --- @return Vector2
    rotated = function(self, angle) return self end,

    --- @param self Vector2
    --- @return number
    --- @return number
    unpack = function(self) return self.x, self.y end,
}

--- @type World
world = setmetatable({}, World)

DEG2RAD = math.pi / 180.0
RAD2DEG = 180.0 / math.pi
