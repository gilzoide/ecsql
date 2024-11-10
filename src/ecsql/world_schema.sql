PRAGMA foreign_keys = ON;

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
CREATE TABLE time(delta, uptime);
INSERT INTO time(delta, uptime) VALUES(0, 0);

-- Screen Size singleton
CREATE TABLE screen_size(width, height);
INSERT INTO screen_size(width, height) VALUES(0, 0);
