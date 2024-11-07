PRAGMA foreign_keys = 1;

CREATE TABLE entity(
  id INTEGER PRIMARY KEY,
  name TEXT,
  parent_id INTEGER REFERENCES entity(id) ON DELETE CASCADE
);
CREATE INDEX entity_name ON entity(name);
CREATE INDEX entity_parent_id ON entity(parent_id);

CREATE VIEW entity_parents AS
  WITH RECURSIVE parents(leaf_id, parent_id, depth) AS (
    SELECT id, parent_id, 1 FROM entity WHERE parent_id IS NOT NULL

    UNION ALL

    SELECT leaf_id, entity.parent_id, depth + 1
    FROM entity JOIN parents ON entity.id = parents.parent_id
    WHERE entity.parent_id IS NOT NULL
  )
  SELECT * FROM parents;

CREATE VIEW entity_children AS
  SELECT parent_id, id FROM entity;

CREATE VIEW entity_nested_children AS
  WITH RECURSIVE children(root_id, child_id, depth) AS (
    SELECT parent_id, id, 1 FROM entity WHERE parent_id IS NOT NULL

    UNION ALL

    SELECT root_id, entity.id, depth + 1
    FROM entity JOIN children ON entity.parent_id = children.child_id
  )
  SELECT * FROM children;

-- Time singleton
CREATE TABLE time(delta, uptime);
INSERT INTO time(delta, uptime) VALUES(0, 0);
