#pragma once

#include <uv.h>
#include <functional>

class Timer {
public:
    typedef std::function<void()> TimeoutHandler;

    static Timer* create(uv_loop_t& loop);
    void destroy();
    bool start(uint64_t timeout, uint64_t repeat, TimeoutHandler&& handler);
    bool again(TimeoutHandler&& handler);
    bool stop();
    uint64_t getRepeat();
    void setRepeat(uint64_t repeat);

private:
    explicit Timer(uv_loop_t& loop);
    static void handleClose(uv_handle_t* handle);
    static void handleTimeout(uv_timer_t* handle);

private:
    uv_loop_t& m_loop;
    uv_timer_t m_handle;
    TimeoutHandler m_handler;
};