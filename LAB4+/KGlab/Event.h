#pragma once

#include <list>
#include <functional>
#include <mutex>

template <class SENDTYPE, class ARGTYPE>
class Event
{
    typedef std::function<void(SENDTYPE, ARGTYPE)> event_func_type;
    typedef typename std::list<event_func_type>::const_iterator event_func_type_const_iterator;

    std::list<event_func_type> events;

    std::mutex event_lock;

  public:
    event_func_type_const_iterator reaction(event_func_type event_func)
    {
        event_lock.lock();
        events.push_back(event_func);
        event_lock.unlock();
        return --events.end();
    }

    template <class C> event_func_type_const_iterator reaction(C* cls, void (C::*f)(SENDTYPE, ARGTYPE))
    {
        using namespace std::placeholders;
        event_func_type f1;
        f1 = std::bind(f, cls, _1, _2);
        event_lock.lock();
        events.push_back(f1);
        event_lock.unlock();
        return --events.end();
    }

    void remove_reaction(event_func_type_const_iterator& it)
    {
        event_lock.lock();
        events.erase(it);
        event_lock.unlock();
    }

    template <class C> void remove_reaction(C* cls, void (C::*f)(SENDTYPE, ARGTYPE))
    {
        using namespace std::placeholders;
        event_func_type f1;
        f1 = std::bind(f, cls, _1, _2);
        event_lock.lock();
        events.remove(f1);
        event_lock.unlock();
    }

    void remove_reaction(event_func_type f)
    {
        event_lock.lock();
        events.remove(f);
        event_lock.unlock();
    }

    void remove_all_reations()
    {
        event_lock.lock();
        events.clear();
        event_lock.unlock();
    }

    void exec(SENDTYPE sender, ARGTYPE args)
    {
        event_lock.lock();
        for (auto x : events)
            x(sender, args);
        event_lock.unlock();
    }
};
