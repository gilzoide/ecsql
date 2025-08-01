PRAGMA foreign_keys = ON;
PRAGMA synchronous = normal;

-- Entity
CREATE TABLE entity(
  id INTEGER PRIMARY KEY,
  name TEXT,
  parent_id INTEGER REFERENCES entity(id) ON DELETE CASCADE
);
CREATE INDEX entity_name ON entity(name);
CREATE INDEX entity_parent_id ON entity(parent_id);

CREATE VIEW entity_parents AS
  WITH RECURSIVE parents(child_id, parent_id) AS (
    SELECT id, parent_id
    FROM entity
    WHERE parent_id IS NOT NULL

    UNION ALL

    SELECT child_id, entity.parent_id
    FROM entity JOIN parents ON entity.id = parents.parent_id
    WHERE entity.parent_id IS NOT NULL
  )
  SELECT * FROM parents;

-- Time singleton
CREATE TABLE time(
  delta DEFAULT 0,
  uptime DEFAULT 0,
  fixed_delta DEFAULT (1.0 / 60.0),
  fixed_delta_progress DEFAULT 0
);
INSERT INTO time DEFAULT VALUES;
