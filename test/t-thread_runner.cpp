#include <xeq/thread_runner.hpp>
#include <doctest/doctest.h>
#include <atomic>

struct fake_ctx {
    std::atomic_int32_t m_counter = 0;
    void run() {
        ++m_counter;
    }
};

TEST_CASE("multi thread runner") {
    fake_ctx ctx;
    {
        xeq::thread_runner runner(ctx, 4, "test");
        CHECK(runner.num_threads() == 4);
        CHECK_FALSE(runner.empty());
    }
    CHECK(ctx.m_counter == 4);
}

TEST_CASE("multi thread runner start/stop") {
    fake_ctx ctx;
    xeq::thread_runner runner;
    CHECK(runner.num_threads() == 0);
    CHECK(runner.empty());

    runner.start(ctx, 3, "three");
    CHECK(runner.num_threads() == 3);
    CHECK_FALSE(runner.empty());
    runner.join();
    CHECK(ctx.m_counter == 3);

    CHECK(runner.num_threads() == 0);
    CHECK(runner.empty());

    runner.start(ctx, 2, "two");
    CHECK(runner.num_threads() == 2);
    CHECK_FALSE(runner.empty());
    runner.join();
    CHECK(ctx.m_counter == 5);

    CHECK(runner.num_threads() == 0);
    CHECK(runner.empty());
}
