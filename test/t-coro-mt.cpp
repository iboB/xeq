#include <xeq/coro.hpp>
#include <xeq/simple_wobj.hpp>
#include <xeq/timer_wobj.hpp>
#include <xeq/context.hpp>
#include <xeq/work_guard.hpp>
#include <xeq/co_spawn.hpp>
#include <xeq/co_execute.hpp>
#include <xeq/thread_runner.hpp>
#include <xeq/strand.hpp>
#include <doctest/doctest.h>

class worker {
    xeq::timer_wobj m_wobj;
    xeq::simple_wobj& m_notify;
public:
    worker(xeq::strand_ptr ex, xeq::simple_wobj& notify)
        : m_wobj(ex)
        , m_notify(notify)
    {
        co_spawn(ex, run());
    }

    int a, b;
    int result;
    xeq::coro<void> run() {
        while (true) {
            auto notified = co_await m_wobj.wait(xeq::timeout::after_ms(100));
            if (!notified) continue;
            result = a + b;
            m_notify.notify_one();
            if (result == 0) co_return;
        }
    }

    void notify() {
        m_wobj.notify_one();
    }
};

xeq::coro<int> test() {
    xeq::context ctx;
    auto wg = ctx.make_work_guard();
    xeq::thread_runner runner(ctx, 1, "worker");

    xeq::simple_wobj wobj(co_await xeq::this_coro::executor{});
    worker wrk(ctx.make_strand(), wobj);

    wrk.a = 5;
    wrk.b = 10;
    wrk.notify();
    CHECK(co_await wobj.wait());
    CHECK(wrk.result == 15);

    wrk.a = 20;
    wrk.b = 30;
    wrk.notify();
    CHECK(co_await wobj.wait());
    CHECK(wrk.result == 50);

    wrk.a = 0;
    wrk.b = 0;
    wrk.notify();
    CHECK(co_await wobj.wait());

    wg.reset();
    runner.join();
    co_return wrk.result;
}

TEST_CASE("worker") {
    CHECK(co_execute(test()) == 0);
}
