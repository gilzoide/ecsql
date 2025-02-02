# Automatically registering components
After some prototyping, I quickly realized that manually registering components is a tedious task.
I was creating components as global C++ instances, then registering them one by one using `world.register_component(component)` in the app's `main` function.

To make it simpler, I created a global linked list that contains references to each and every Component instance:
```cpp
class Component {
private:
    // 1. Declare the linked list
    inline static std::forward_list<Component *> static_linked_list;

public:
    // 2. Insert each Component to the list in its constructor
    Component(...) {
      static_linked_list.push_front(this);
    }

    // 3. Add a "foreach" method that applies a function to all instances
    template<typename Fn>
    static void foreach(Fn&& f) {
        for (auto it : static_linked_list) {
            f(it);
        }
    }
};
```

Then registered all components using the `foreach` function:
```cpp
ecsql::Component::foreach([&](ecsql::Component *component) {
    world.register_component(*component);
});
```

And that's it!
Now, to create new components, all we need to do is define it in a C++ file, and they will be registered automatically!
```cpp
// e.g. in my_component.cpp
ecsql::Component MyComponent {
    "MyComponent",
    {
        "field1",
        "field2",
    },
};

// Now MyComponent is in the linked list and will be registered automatically
```


## Systems
One thing to notice about global variables in C++ is that they are constructed in the order they are declared in a single translation unit (each .cpp file is a translation unit most of the time), but the order is undefined between different translation units.

That means we can only use this trick safely if the order of construction is not relevant.
components can be registered pretty much in any order, so it's ok to use a global linked list for this.
For systems, on the other hand, it may be desirable to define the registration order, since they are run in the same order as they are registered.
For that reason, I didn't use the same trick for systems, which are currently registered manually.


## HookSystems
Hook systems, on the other hand, can also be registered in any order.
I don't think I'll ever have a requirement on hook system execution order, so I did the same trick for it.


## Conclusion
That's it, I worked in this to save me time in future work.
Cheers \o/
