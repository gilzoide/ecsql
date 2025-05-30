system "UpdateUptimeStatistics" {
    [[
        UPDATE save.statistics
        SET total_uptime = total_uptime + time.delta
        FROM time
    ]],
    [[
        SELECT total_uptime
        FROM save.statistics
    ]],
    [[
        SELECT entity_id, text
        FROM UptimeText
            JOIN Text USING(entity_id)
    ]],
    [[
        UPDATE Text
        SET text = ?2
        WHERE Text.entity_id = ?1
    ]],
    function(update_uptime, get_uptime, select_texts, update_text)
        update_uptime()
        local uptime_text
        for row in select_texts() do
            local entity_id, current_text = row:unpack()
            if not uptime_text then
                local pieces = {}
                local uptime = get_uptime():unpack()
                while uptime > 0 do
                    table.insert(pieces, 1, string.format("%02d", math.floor(uptime % 60)))
                    uptime = uptime // 60
                end
                uptime_text = table.concat(pieces, ":")
            end
            if current_text ~= uptime_text then
                update_text(entity_id, uptime_text)
            end
        end
    end
}
