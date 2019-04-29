#include "timer.h"
#include "util.h"

Timer::Timer(uv_loop_t& loop) : m_loop(loop) {
    uv_timer_init(&m_loop, &m_handle);
}

// static
Timer* Timer::create(uv_loop_t& loop) {
    return new Timer(loop);
}

void Timer::destroy() {
    uv_close((uv_handle_t*)&m_handle, handleClose);
}

// static
void Timer::handleClose(uv_handle_t* handle) {
    Timer* timer = CONTAINER_OF(handle, Timer, m_handle);
    delete timer;
}

bool Timer::start(uint64_t timeout, uint64_t repeat, TimeoutHandler&& handler) {
    m_handler = std::move(handler);
    int retval = uv_timer_start(&m_handle, handleTimeout, timeout, repeat);
    if (retval != 0) {
        return false;
    }
    return true;
}

// static 
void Timer::handleTimeout(uv_timer_t* handle) {
    Timer* timer = CONTAINER_OF(handle, Timer, m_handle);
    timer->m_handler();
}

bool Timer::again(TimeoutHandler&& handler) {
    int retval = uv_timer_again(&m_handle);
    if (retval != 0) {
        return false;
    }
    m_handler = std::move(handler);
    return true;
}

bool Timer::stop() {
    int retval = uv_timer_stop(&m_handle);
    if (retval != 0) {
        return false;
    }
    return true;
}

uint64_t Timer::getRepeat() {
    return uv_timer_get_repeat(&m_handle);
}

void Timer::setRepeat(uint64_t repeat) {
    return uv_timer_set_repeat(&m_handle, repeat);
}