component "BakePosition" {
    "parent_id INTEGER REFERENCES entity(id)",
    "x NOT NULL DEFAULT 0",
    "y NOT NULL DEFAULT 0",
    "z NOT NULL DEFAULT 0",
}
