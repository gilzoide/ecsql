sql([[
CREATE TABLE IF NOT EXISTS save.schema_version(version INTEGER);
INSERT OR IGNORE INTO save.schema_version(rowid, version) VALUES(1, 0);
]])

local MIGRATIONS = {
    [[
        CREATE TABLE save.statistics(total_uptime FLOAT DEFAULT 0);
        INSERT INTO save.statistics DEFAULT VALUES;
    ]],
}

local current_version = sql("SELECT version FROM save.schema_version")[1]
if current_version < #MIGRATIONS then
    if DEBUG and current_version > 0 then
        local backup_name = string.format("save-migration%02d.sqlite3.bkp", current_version)
        print("Backing up save before migration: " .. backup_name)
        world:backup_into(backup_name, "save")
    end
    for i = current_version + 1, #MIGRATIONS do
        sql(MIGRATIONS[i])
    end
    sql("UPDATE save.schema_version SET version = ?", #MIGRATIONS)
end
