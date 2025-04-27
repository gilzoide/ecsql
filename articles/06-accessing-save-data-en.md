# Accessing save data
> 2025-04-21 | `#ECS #SQL #SQLite #C++ #SaveData #Migration` | [*Versão em Português*](06-accessing-save-data-pt.md)

There's no better tool to save a player's progress than a database.
We already have a SQLite database connection to store our ECS data, so we'll also use it to store the save data.


## Attaching the save database
One of the many cool things about SQLite is that it supports [attaching multiple databases to a single connection](https://sqlite.org/lang_attach.html).
With that feature, we can access data in the save database using the same interface that we use to access ECS data.
We can even join tables from one database with the other, making it possible to implement systems that operate on both databases at the same time.

To attach the save database, all we need to do is run an `ATTACH` SQL statement such as the following:
```sql
ATTACH DATABASE 'save.sqlite3' AS save;
```
SQLite will automatically create the database file if it doesn't exist yet.

After attaching the database, we can use the prefix `save.` to specify tables that refer to the newly attached `save` database.
Here's an example table with gameplay statistics that will be stored in the save database:
```sql
CREATE TABLE save.statistics(
  total_uptime FLOAT DEFAULT 0
);
```

Now, we can use tables from the `save` database in our systems.
Here's an example query that stores the total uptime, which gets persisted between sessions in the save database:
```sql
UPDATE save.statistics
SET total_uptime = total_uptime + time.delta
FROM time;
```
Notice that the `time` table belongs to the ECS database, while `save.statistics` refers to the save database.

> I have written more about [how ECSQL tracks time here](04-tracking-time-en.md), check it out if you haven't yet.


## Database migrations
As we develop and release new versions of our game, we'll often need to change the structure of the save database over time.
Since this data is persisted between sessions, we need a way to change the database without losing previous data.

One common approach to this problem is defining database migration scripts.
We keep a list of migration scripts and run them in order when the game starts if they haven't been run yet.
To keep track of which scripts have been run, we store a version number in the database.
Here's how the schema for the version bookkeeping looks like:
```sql
-- The schema version table simply stores the current version
CREATE TABLE IF NOT EXISTS save.schema_version(version INTEGER);

-- Using "OR IGNORE" and a specific "rowid" makes sure we add the
-- row in the first time, but never modify it afterwards
INSERT OR IGNORE INTO save.schema_version(rowid, version) VALUES(1, 0);
```

So by default, when version is 0, no migrations have been run yet.
In C++ code, we check which version is the current and run the missing migrations:
```cpp
int version = world.execute_sql(R"(
    SELECT version
    FROM save.schema_version
)").get<int>();

if (version < 1) {
    // Run migration 1
    // For example, our "statistics" table mentioned above
    world.execute_sql_script(R"(
        CREATE TABLE save.statistics(
          total_uptime FLOAT DEFAULT 0
        );
        INSERT INTO save.statistics DEFAULT VALUES;

        -- Update current version
        UPDATE save.schema_version
        SET version = 1
    )");
}

if (version < 2) {
    // Run migration 2
    // We decided we also want to keep session count in "statistics"
    world.execute_sql_script(R"(
        ALTER TABLE save.statistics ADD COLUMN session_count INTEGER DEFAULT 0;

        -- Update current version
        UPDATE save.schema_version
        SET version = 2
    )");
}

// ...
```

As well as creating and altering tables, we can create/alter indexes, constraints, triggers, etc.


## Conclusion
We saw how we can save player's data using a SQLite database and how to access it in tandem with ECS data in our systems.
We also saw a simple way to apply and track down database migrations.

I hope you saved that information in your memory, see you next time!
