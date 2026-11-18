#pragma once
#include <condition_variable>
#include <deque>
#include <mutex>
#include <utility>

namespace Qfr {
// Writer never takes a graphics lock. Producers may block; failed/closed queues wake all producers.
template<class T> class Queue {
    std::mutex mutex; std::condition_variable readable,writable;
    std::deque<std::pair<T,size_t>> items; size_t bytes=0,limit; bool closed=false,failed=false;
public:
    size_t high=0; unsigned long long stalls=0;
    explicit Queue(size_t capacity):limit(capacity){}
    bool push(T value,size_t size) {
        std::unique_lock lock(mutex);
        if(size>limit)return false;
        if(!closed && !failed && bytes+size>limit)++stalls;
        writable.wait(lock,[&]{return closed || failed || bytes+size<=limit;});
        if(closed || failed)return false;
        bytes+=size;if(bytes>high)high=bytes;items.emplace_back(std::move(value),size);readable.notify_one();return true;
    }
    // Routine opening playtests must never freeze the graphics thread waiting
    // for diagnostics. A reserve leaves space for errors and state changes.
    bool tryPush(T value,size_t size,size_t budget) {
        std::lock_guard lock(mutex);const auto cap=budget<limit?budget:limit;
        if(closed||failed||size>cap||bytes>cap-size)return false;
        bytes+=size;if(bytes>high)high=bytes;items.emplace_back(std::move(value),size);readable.notify_one();return true;
    }
    bool pop(T& value) {
        std::unique_lock lock(mutex);
        readable.wait(lock,[&]{return closed || failed || !items.empty();});
        if(items.empty())return false;
        value=std::move(items.front().first);bytes-=items.front().second;items.pop_front();writable.notify_all();return true;
    }
    void close(){std::lock_guard lock(mutex);closed=true;readable.notify_all();writable.notify_all();}
    void fail(){std::lock_guard lock(mutex);failed=true;readable.notify_all();writable.notify_all();}
};
}
