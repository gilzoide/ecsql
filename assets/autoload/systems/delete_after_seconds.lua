system "DeleteAfterSeconds" {
    [[
        WITH entities_to_delete AS (
            SELECT entity_id
            FROM DeleteAfter
                JOIN time
            WHERE uptime - create_uptime >= seconds
        )
        DELETE FROM entity
        WHERE id IN entities_to_delete
    ]],
}
