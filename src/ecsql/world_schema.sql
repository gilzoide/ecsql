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
CREATE TABLE time(delta, uptime, fixed_delta);
INSERT INTO time(delta, uptime, fixed_delta) VALUES(0, 0, 1.0 / 60.0);

-- Screen singleton
CREATE TABLE screen(
  width, height,  -- screen size
  r, g, b, a  -- screen clear color
);
INSERT INTO screen(width, height) VALUES(0, 0);

-- Input (for now, only keyboard is supported)
CREATE TABLE keyboard(
  key INTEGER PRIMARY KEY,
  name,
  state,  -- one of: NULL, 'pressed', 'hold', 'released'
  is_down AS (COALESCE(state IN ('pressed', 'hold'), FALSE))
);
CREATE INDEX keyboard_state ON keyboard(state);
CREATE INDEX keyboard_name_state ON keyboard(name, state);

CREATE TABLE input_map(
  action,
  input
);
CREATE UNIQUE INDEX input_map_action_input ON input_map(action, input);

CREATE TABLE input_action(
  action TEXT PRIMARY KEY,
  state,
  is_down AS (COALESCE(state IN ('pressed', 'hold'), FALSE))
);
