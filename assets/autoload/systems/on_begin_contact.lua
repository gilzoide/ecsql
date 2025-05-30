system "OnBeginContact" {
    [[
        SELECT
            OnBeginContact.entity_id, callback,
            shape1, shape2
        FROM OnBeginContact
            JOIN Contact ON (Contact.shape1 = OnBeginContact.entity_id OR Contact.shape2 = OnBeginContact.entity_id)
    ]],
    function(select_callbacks)
        for row in select_callbacks() do
            local entity_id, callback, shape1, shape2 = row:unpack()
            local other_id = shape1 == entity_id and shape2 or shape1
            require(callback)(entity_id, other_id)
        end
    end
}
