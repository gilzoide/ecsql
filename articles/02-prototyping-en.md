# Starting the prototype: a world with entities, components and systems
> 2024-12-15 | `#ECS #SQL #SQLite` | [*Versão em Português*](02-prototyping-pt.md)

In the [previous article](01-ecs-databases-en.md) I showed you how we can model ECS worlds as relational databases.
Today I'll go through some of the design choices for the prototype I already started creating.


## Base tech stack
- C++: a language that I know well enough how to use and that has tons of ready-to-use libraries for software/game development.
  I'm using the C++20 standard in this project.
- [CMake](https://cmake.org/) for the builds.
- [SQLite](https://sqlite.org/) for the SQL database engine.
- [raylib](https://www.raylib.com/) as the game frontend library.
  Super easy to use, easy to build, supports several platforms, simply an awesome library.


## Entities
In ECSQL, entities are represented by numeric IDs.
There's the `EntityID` type alias for `sqlite3_int64`, which is the underlying type in SQLite, and that's basically it.
```cpp
namespace ecsql {
    typedef sqlite3_int64 EntityID;
}
```


## Components
In ECSQL each component type is a table in the database.
There's a `Component` class with the component's name, a list of fields and an optional additional SQL.
SQLite uses [flexible typing](https://sqlite.org/flextypegood.html), so specifying a field's type is optional.
You can also add "DEFAULT" values, "CHECK" "NOT NULL" and "REFERENCES" constraints, anything that's valid in column definitions in SQLite is valid here.
The optional additional SQL may be used to declare indices, triggers and views, among other things.

Example:
```cpp
ecsql::Component PositionComponent {
    // name
    "Position",
    // fields
    {
        "x DEFAULT 0",
        "y DEFAULT 0",
        "z DEFAULT 0",
    },
    // (optional) additional SQL
    "",
};
```


## Systems
In ECSQL systems are functions that operate on entities and components in the world.
Systems have a name, a list of SQL statements and an implementation.
The implementation receives the World and a list of prepared SQL statements.
Prepared SQL statements are cached between calls, so that we only spend time preparing them once per system.

Example:
```cpp
ecsql::System DrawPointSystem {
    // name
    "DrawPoint",
    // SQL statements
    {
        R"(
          SELECT
              x, y, z,
              r, g, b, a
          FROM PointTag
              JOIN Position USING(entity_id)
              JOIN Color USING(entity_id)
        )",
    },
    // implementation
    [](ecsql::World& world, std::vector<ecsql::PreparedSQL>& prepared_sqls) {
        auto select_points_to_draw = prepared_sqls[0];
        for (ecsql::SQLRow row : select_points_to_draw()) {
            auto [position, color] = row.get<Vector3, Color>();
            draw_point(position, color);
        }
    },
};
```


## Hook Systems
Hook Systems use [SQLite preupdate hooks](https://www.sqlite.org/c3ref/preupdate_blobwrite.html) and are called when rows are inserted/updated/deleted.
They are used to bridge SQL data with native data, so that native data can be created/updated/deleted whenever their corresponding SQL rows do.

> I've written more about how I connect SQL data with C++ data [here](05-flyweight-resources-en.md)

Example:
```cpp
ecsql::HookSystem PositionHook {
    // Component name
    "Position"
    // implementation
    [](ecsql::HookType hook, ecsql::SQLBaseRow& old_row, ecsql::SQLBaseRow& new_row) {
        switch (hook) {
            case ecsql::HookType::OnInsert:
                // Position inserted
                break;

            case ecsql::HookType::OnUpdate:
                // Position updated
                break;

            case ecsql::HookType::OnDelete:
                // Position deleted
                break;
      }
    },
};
```


## World
Finally, the ECS World.
The world contains the main database connection, the list of registered systems and hook systems.
There are methods to create entities, register components and systems, as well as an `update` method that runs all registered systems.

Example:
```cpp
ecsql::World world;
world.register_component(PositionComponent);
world.register_system(DrawPointSystem);
world.register_hook_system(PositionHook);

while (game_is_running()) {
    float delta_time = get_delta_time();
    world.update(delta_time);
}
```


## Conclusion
That's the base architecture for the ECSQL project.
In the following articles I'll show more specific information on problems and solutions for the ECSQL implementation.
Until next time!
