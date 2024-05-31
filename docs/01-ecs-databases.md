# Designing ECSQL

## Primer on ECS
Entity Component System, or ECS for short, is a design pattern that provides code reusability by separating data from behaviour logic, often used by video games.
ECS models are composed of:
- Entities: objects living in the world, represented by unique identifiers.
- Components: data types that can be attached to entities.
  An entity can have zero or more components, which can be added and removed from them dynamically.
  For example a "Position" component used by objects in a 3D space could have 3 float properties named "x", "y" and "z".
- Systems: functions that implement behavior logic, applied selectively to entities that have a specific set of components.
  For example, a "Move" system that moves entites in a 3D space could be applied selectively to entities that have both a "Position" and "Velocity" components.
- World: the container for all ECS data, such as living entities and components.
  ECS implementations often support creating and processing multiple worlds at the same time.

For more detailed explanation of ECS concepts, check out this [ECS FAQ](https://www.flecs.dev/ecs-faq/).


## ECS and databases
An ECS world can be viewed as a database, which stores all living entities and their components data.
The world is queried to match systems with the correct component set, using a sort of JOIN operation between different components.
Components may be added and removed from their entities in either a one-to-one or one-to-many relationship.

Thinking about this analogy of ECS worlds as databases, what if we implemented an ECS framework using SQL databases, powered by [SQLite](https://sqlite.org)?

The pros:
- With SQL we can not only query for specific components, but we can further filter data based on their values.
  For example, we can query for specific enumeration values inside components to implement state machines.
- We can use an in-memory database or operate directly to files in the filesystem.
  Using files will likely be slower, since they require I/O with the filesystem, but this feature might be useful in some cases.
- We can load and save all data to files using [SQLite's Backup API](https://www.sqlite.org/backup.html), easily implementing quick save/quick load features.
- Since world data is queried using SQL, we can implement systems in different progamming languages and all of them can operate on the same data, all it takes is an open connection to the database.

The cons:
- Even if we avoid disk I/O using an in-memory database, querying with SQLite will be slower than the specialized data structures used by other ECS frameworks, possibly an order of magnitude slower.
  One of the benefits of ECS is that it's possible to layout data in a cache-friendly manner, so that iterating on components is very very fast.
- Some types of data, like pointers to native structures, don't have a nice representation in SQLite and might need an additional indirection when stored as component data.

Even if it will certainly be slower than specialized ECS implementations, SQLite is very fast in general and it might be fast enough for some game projects, so I want to experiment with this idea and see how far it goes.
This experimental ECS framework powered by SQLite will be called **ECSQL**.


## Representing entities in SQL
In ECS, entities are represented by a unique identifier, usually an integer value.
To represent entities in SQL, all we need to do is create an `entity` table that stores the existing entities, represented by their numeric ids:
```sql
CREATE TABLE entity (
  id INTEGER PRIMARY KEY
);
```


## Representing components in SQL
Components are data blocks that can be attached to entities.
All components must have an associated entity.
Also when the entity is deleted, its components should be deleted along with it.

To me, the best representation for ECS components in SQL is creating a table for each.
All components have their own ids, and their owner entity's id.
When querying component sets for a system, all we need to do is `JOIN` all these tables together using the entity id.

The base for every component could be defined like the following:
```sql
CREATE TABLE component (
  -- Component's own id, used for updating its data from systems
  id INTEGER PRIMARY KEY,

  -- Owner entity id
  -- "NOT NULL": components must always be associated with an entity
  -- "REFERENCES entity(id)": foreign key constraint
  -- "ON DELETE CASCADE": delete this component when entity is deleted
  entity_id INTEGER NOT NULL REFERENCES entity(id) ON DELETE CASCADE
);
```

Since we'll want to match components using the entity id, let's also add an index to it for faster queries:
```sql
CREATE INDEX component_entity_id ON component(entity_id);
```

For example, a 3D "Position" component could be defined like the following table:
```sql
CREATE TABLE position (
  id INTEGER PRIMARY KEY,
  entity_id INTEGER NOT NULL REFERENCES entity(id) ON DELETE CASCADE,

  -- 3D position axes, all defaulting to 0
  x FLOAT DEFAULT 0,
  y FLOAT DEFAULT 0,
  z FLOAT DEFAULT 0
);

-- Don't forget the index on "entity_id"
CREATE INDEX position_entity_id ON position(entity_id);
```

As another example, a "Velocity" component could be defined like so:
```sql
CREATE TABLE velocity (
  id INTEGER PRIMARY KEY,
  entity_id INTEGER NOT NULL REFERENCES entity(id) ON DELETE CASCADE,

  -- Velocity value, in m/s
  value FLOAT DEFAULT 0
);
CREATE INDEX velocity_entity_id ON velocity(entity_id);
```


## Querying components from systems
Systems by themselves will not live in the SQLite database, but they will use SQL for querying components.
Let's take the "Move" system example one more time, which is applied to entities with both a "Position" and "Velocity" components as described above.
Iterating over all entities that have both components can be accomplished with the following query:
```sql
SELECT 
  entity_id,
  position.id, position.x, position.y, position.z,
  velocity.id, velocity.value
FROM position INNER JOIN velocity USING(entity_id);
```

We could also easily support systems that require some components, but only optionally requires others.
This can be done by simply changing the `INNER JOIN` by an `OUTER LEFT|RIGHT JOIN`, making SQLite return `NULL` for data from components that are not present in the entity.
