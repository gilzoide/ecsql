# Tracking time
> 2025-02-04 | `#ECS #SQL #SQLite #C++ #GameLoop #DeltaTime` | [*Versão em Português*](04-tracking-time-pt.md)

One thing you'll see in pretty much any game engine is that they all have a way to get the time elapsed since the last frame, the so called "delta time".
This is used to convert between frames and real world time, making games run in the same speed independently of how many frames are processed per second.

For example, if a sprite moves 10 pixels each frame, it will move at the speed of 600 pixels per second if the game is running at 60 frames per second (FPS), but only 300 pixels/s if runing at 30 FPS.
The game may run at 60 FPS on a high-end device, but only at 30 FPS on a lower-end device, and we need should make the game experience the same in both cases.
To ensure a consistent gameplay experience, we should move the sprite in a rate of pixels per second instead of pixels per frame, that's where delta time come in handy.


## Storing time in SQL
We'll store the current frame's delta time and total uptime in SQL to make this information usable by systems.

We'll create the singleton `time` table.
"Singleton" here means that this table will only ever have a single row.
Its definition is the following:
```sql
CREATE TABLE time(
  delta,
  uptime
);

-- Here's the singleton row
-- No time has passed yet, so zero both values
INSERT INTO time(delta, uptime) VALUES(0, 0);
```

Now in the game loop, we fetch the current delta time using raylib's `GetFrameTime()` and pass it to the following UPDATE query:
```sql
UPDATE time
SET delta = ?1, uptime = uptime + ?1;
```

Notice that we are using `?1` for updating both `delta` and `uptime`, meaning this variable will receive the same value.
This makes `uptime` be always incremented by `delta` each frame, so it will always contain the sum of `delta` times of all frames, resulting in the total time that the game has been running.


## Using delta time in systems
Ok, let's get for example a movement system that moves an entity's `Position` with constant speed, represented by `LinearVelocity`:
```sql
UPDATE Position
SET
  x = Position.x + LinearVelocity.x,
  y = Position.y + LinearVelocity.y,
  z = Position.z + LinearVelocity.z
FROM LinearVelocity
WHERE Position.entity_id = LinearVelocity.entity_id
```

To make this query use the delta time to move the entity with the same speed independent of frame rate, all we need to do is join the `time` table and multiply the `LinearVelocity` values by `time.delta`:
```sql
UPDATE Position
SET
  x = Position.x + LinearVelocity.x * time.delta,
  y = Position.y + LinearVelocity.y * time.delta,
  z = Position.z + LinearVelocity.z * time.delta
FROM LinearVelocity JOIN time
WHERE Position.entity_id = LinearVelocity.entity_id
```


## Using uptime in systems
Having the total uptime is useful when we want time-based behaviors.

For example, let's make a system that deletes an entity after some time, specified by the component `DeleteAfter`:
```sql
-- CREATE TABLE DeleteAfter(
--   entity_id INTEGER PRIMARY KEY REFERENCES ...,
--   seconds,
--   inserted_uptime
-- );

WITH entities_to_delete AS (
  SELECT entity_id
  FROM DeleteAfter JOIN time
  WHERE time.uptime - DeleteAfter.inserted_uptime >= DeleteAfter.seconds
)
DELETE FROM entity
WHERE id IN entities_to_delete;
```


## Bonus content: enforcing singleton table
Ok, so this is not really related to game loop and delta time.
I'm going to show you how to enforce that no one ever inserts a new row into the singleton `time` table.

> Note: I didn't do this in ECSQL, I'm showing you this just for fun.

The best way to do this is using [triggers](https://www.sqlite.org/lang_createtrigger.html), which are database operations that run when a row is either inserted, updated or deleted from a table.
One of the things we can do with triggers is abort the query that triggered the trigger-program:
```sql
CREATE TRIGGER abort_insert_time_singleton
BEFORE INSERT ON time
BEGIN
  SELECT RAISE(ABORT, 'Inserting rows in time table is forbidden');
END;

CREATE TRIGGER abort_delete_time_singleton
BEFORE DELETE ON time
BEGIN
  SELECT RAISE(ABORT, 'Deleting rows in time table is forbidden');
END;
```

Now trying to insert into or delete from the `time` table will fail with the message we supplied:
```sql
INSERT INTO time DEFAULT VALUES;
-- Error: 'Inserting rows in time table is forbidden'

DELETE FROM time;
-- Error: 'Deleting rows in time table is forbidden'
```


## Conclusion
Time is always running, and we need to keep track of it in our game engine.
See you next time!
