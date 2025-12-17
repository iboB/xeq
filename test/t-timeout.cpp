#include <xeq/timeout.hpp>
#include <doctest/doctest.h>

TEST_CASE("timeout") {
    using xeq::timeout;
    {
        timeout t(std::chrono::milliseconds(42));
        CHECK(t.ms() == 42);
        CHECK(t.is_finite());
        CHECK_FALSE(t.is_infinite());
        CHECK_FALSE(t.is_zero());
    }

    {
        auto t = timeout::after(std::chrono::milliseconds(12));
        CHECK(t.ms() == 12);
        CHECK(t.is_finite());
        CHECK_FALSE(t.is_infinite());
        CHECK_FALSE(t.is_zero());
    }

    {
        auto t = timeout::after_ms(8);
        CHECK(t.ms() == 8);
        CHECK_FALSE(t.is_infinite());
        CHECK_FALSE(t.is_zero());
    }

    {
        auto t = timeout::never();
        CHECK(t.ms() < 0);
        CHECK_FALSE(t.is_finite());
        CHECK(t.is_infinite());
        CHECK_FALSE(t.is_zero());
    }

    {
        auto t = timeout::now();
        CHECK(t.ms() == 0);
        CHECK(t.is_finite());
        CHECK_FALSE(t.is_infinite());
        CHECK(t.is_zero());
    }

    {
        auto t = timeout::immediately();
        CHECK(t.ms() == 0);
        CHECK_FALSE(t.is_infinite());
        CHECK(t.is_zero());
    }

    {
        timeout t(std::chrono::seconds(4));
        CHECK(t.ms() == 4000);
    }

    {
        timeout t(std::chrono::microseconds(3212));
        CHECK(t.ms() == 4);
    }

    {
        auto t = timeout::after(std::chrono::microseconds(9876));
        CHECK(t.ms() == 10);
    }

    {
        timeout t(std::chrono::nanoseconds(0));
        CHECK(t.ms() == 0);
        CHECK(t.is_zero());
    }
}

TEST_CASE("chrono") {
    using namespace std::chrono_literals;
    using xeq::timeout;

    timeout t = 42ms;
    CHECK(t.ms() == 42);

    t = timeout::after(34ms);
    CHECK(t.ms() == 34);

    t = timeout::after(21343us);
    CHECK(t.ms() == 22);

    t = timeout::after(2s);
    CHECK(t.ms() == 2000);

    t = timeout::after(-3min);
    CHECK(t.is_infinite());

    t = timeout::after(0ms);
    CHECK(t.is_zero());
}



TEST_CASE("cmp") {
    using xeq::timeout;
    timeout a1 = timeout::after_ms(10);
    timeout a2 = timeout::after_ms(20);
    timeout b1 = timeout::after_ms(10);

    CHECK(a1 < a2);
    CHECK(a2 > a1);
    CHECK(a1 <= b1);
    CHECK(a1 >= b1);
    CHECK(a1 == b1);
    CHECK(a1 != a2);

    timeout inf = timeout::never();
    CHECK(inf > a1);
    CHECK(a1 < inf);
}

TEST_CASE("vals") {
    using namespace xeq::timeout_vals;
    CHECK(await_completion_for(std::chrono::milliseconds(53)).ms() == 53);
    CHECK(await_completion.is_infinite());
    CHECK(no_wait.is_zero());
    CHECK(proceed_immediately.is_zero());
}
