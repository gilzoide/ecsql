# External resources and the Flyweight pattern
> 2025-02-09 | `#ECS #SQLite #C++ #Flyweight` | [*Versão em Português*](05-flyweight-resources-pt.md)

One of the problems that quickly arise from using SQLite as a data store for ECS is: how to associate native C++ data with SQL data?

Example: to draw textures using raylib we need an instance of the `Texture` struct.
But systems query components using SQL, how to get the `Texture` instance to be drawn?

To solve this problem, we'll need to maintain a map between SQL data and native instances.
Each native instance will have an identifier, for example the associated entity's numeric identifier.
In the system implementation we join the SQL data with native data using C++ code by querying this map.
A C++ `std::map` or `std::unordered_map` is enough for implementing this.


## Mapping SQL to textures
Let's take for example a `Texture` component that contains the asset path to be loaded:
```cpp
ecsql::Component TextureComponent {
    "Texture",
    {
        "asset_path TEXT",
    }
};
```

We'll create the map that will store native `Texture` instances for each entity:
```cpp
std::unordered_map<ecsql::EntityId, Texture> TextureMap;
```

In the system's implementation, we query the texture map using the identifier got from the SQL query:
```cpp
ecsql::System DrawTextureAtOrigin {
    // System name
    "DrawTextureAtOrigin",
    // SQL statements
    {
        "SELECT entity_id FROM Texture",
    },
    // implementation
    [](ecsql::World& world, std::vector<ecsql::PreparedSQL>& prepared_sqls) {
        auto select_texture = prepared_sqls[0];
        for (ecsql::SQLRow row : select_texture()) {
            auto entity_id = row.get<ecsql::EntityId>();
            auto it = TextureMap.find(entity_id);
            if (it != TextureMap.end()) {
                Texture texture = it->second();
                DrawTexture(texture, 0, 0, WHITE);
            }
        }
    },
};
```

All that's left to do is actually load the texture into the map when it's needed and unload it when not needed anymore.
We'll use a hook system for this, so that we load the texture automatically when inserting a row in the `Texture` table and unload it when the row gets deleted:
```cpp
ecsql::HookSystem TextureMapHook {
    // Component name
    "Texture"
    // implementation
    [](ecsql::HookType hook, ecsql::SQLBaseRow& old_row, ecsql::SQLBaseRow& new_row) {
        switch (hook) {
            case ecsql::HookType::OnInsert: {
                auto [entity_id, asset_path] = new_row.get<ecsql::EntityId, const char *>();
                Texture texture = LoadTexture(asset_path);
                TextureMap.emplace(entity_id, texture);
                break;
            }

            case ecsql::HookType::OnDelete: {
                auto entity_id = old_row.get<ecsql::EntityId>();
                auto it = TextureMap.find(entity_id);
                if (it != TextureMap.end()) {
                    UnloadTexture(it->second());
                    TextureMap.erase(it);
                }
                break;
            }
        }
    },
};
```

Now our `DrawTextureAtOrigin` system should be able to draw textures successfully.


## Reusing duplicated resources
One thing that you'll notice is that if we use the same texture asset for several entities, it will be loaded several times and we'll end up with lots of duplicates, one for each entity with a Texture component.
Instead of loading the same texture several times, we want to have a single `Texture` instance for each loaded texture asset and share it between all entities that use the same asset.
This idea of sharing data with similar objects to reduce memory usage is the core of the [Flyweight](https://en.wikipedia.org/wiki/Flyweight_pattern) design pattern.

Now, instead of mapping entity IDs to `Texture` instances, we'll map them based on the asset path.
If 100 entities use a texture asset with the same path, all of them will share a single native instance.
This not only makes memory usage lower, but helps with reducing GPU usage by helping draw batching (which I'm not going to cover in this article).

To make sure that we unload the texture when it is not needed anymore by any entity, we'll keep a reference count for each texture.
When there's currently 0 entities referencing a texture and we insert a new row, we'll load the texture and mark the reference count as 1.
As more rows are inserted, we keep incrementing the count without loading the texture again.
As rows are deleted, we decrement the count.
Whenever we delete the last row, when the count is going from 1 to 0, this is when we finally unload the texture.

I'll be using my own [flyweight.hpp](https://github.com/gilzoide/flyweight.hpp) package for the Flyweight implementation, since it has support for automatic reference counting.
So, instead of a `std::unordered_map` of entity ID to Texture, we now have a reference-counted Flyweight that maps asset paths (`std::string`) to Texture:
```cpp
flyweight::flyweight_refcounted<std::string, Texture> TextureFlyweight {
    // Object loader, called when reference count goes from 0 to 1
    [](const std::string& asset_path) {
      return LoadTexture(asset_path.c_str());
    },
    // Object unloader, called when reference count goes from 1 to 0
    [](Texture& texture) {
        UnloadTexture(texture);
    }
}
```

The hook system now looks like this:
```cpp
ecsql::HookSystem TextureMapHook {
    // Component name
    "Texture"
    // implementation
    [](ecsql::HookType hook, ecsql::SQLBaseRow& old_row, ecsql::SQLBaseRow& new_row) {
        switch (hook) {
            case ecsql::HookType::OnInsert: {
                auto asset_path = new_row.get<std::string>(1);
                // `get` increments the reference count
                // The texture will only get loaded when going from 0 to 1
                TextureFlyweight.get(asset_path);
                break;
            }

            case ecsql::HookType::OnDelete: {
                auto asset_path = old_row.get<std::string>(1);
                // `release` decrements the reference count
                // The texture will only get unloaded when going from 1 to 0
                TextureFlyweight.release(asset_path);
                break;
            }
        }
    },
};
```

And the `DrawTextureAtOrigin` system now goes like this:
```cpp
ecsql::System DrawTextureAtOrigin {
    // System name
    "DrawTextureAtOrigin",
    // SQL statements
    {
        "SELECT asset_path FROM Texture",
    },
    // implementation
    [](ecsql::World& world, std::vector<ecsql::PreparedSQL>& prepared_sqls) {
        auto select_texture = prepared_sqls[0];
        for (ecsql::SQLRow row : select_texture()) {
            auto asset_path = row.get<std::string>();
            // `peek` returns a pointer to the texture, if it is loaded
            // Since we have the hook system, it will always be loaded!
            Texture *texture = TextureFlyweight.peek(asset_path);
            DrawTexture(*texture, 0, 0, WHITE);
        }
    },
};
```


## Conclusion
Not everything necessary for running the game can be stored in the database, but that's ok.
We can always bridge the SQL data with native C++ data using associative maps.

We've also seen how to avoid duplicating native resources by using the Flyweight design pattern, which is a really common feat in game engines.

That's about it, hope you liked it!

Cheers \o/
