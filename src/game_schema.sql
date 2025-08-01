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

CREATE TABLE input_action_axis(
  action TEXT PRIMARY KEY,
  action_positive,
  action_negative,
  value
);
