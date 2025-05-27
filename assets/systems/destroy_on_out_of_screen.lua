system "DestroyOnOutOfScreen" {
    [[
        DELETE FROM entity
        WHERE id IN (
            SELECT entity_id
            FROM DestroyOnOutOfScreen
                JOIN Position USING(entity_id)
                JOIN screen
            WHERE position.x NOT BETWEEN 0 AND screen.width
                OR position.y NOT BETWEEN 0 AND screen.height
        )
    ]],
}
