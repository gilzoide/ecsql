#pragma once

#include <forward_list>

#define STATIC_LINKED_LIST_DEFINE(type) \
private: \
    inline static std::forward_list<type *> static_linked_list; \
public: \
    template<typename Fn> \
    static void foreach_static_linked_list(Fn&& f) {\
        for (auto it : static_linked_list) {\
            f(it); \
        } \
    }

#define STATIC_LINKED_LIST_INSERT() \
	static_linked_list.push_front(this)
