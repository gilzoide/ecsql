component "DeleteAfter" {
    "seconds NOT NULL",
    "create_uptime",
    additional_schema = [[
        CREATE TRIGGER DeleteAfter_inserted
        AFTER INSERT ON DeleteAfter
        BEGIN
            UPDATE DeleteAfter
            SET create_uptime = uptime
            FROM time
            WHERE entity_id = new.entity_id;
        END;
    ]]
}
