#include <xeq/coro.hpp>
#include <xeq/co_spawn.hpp>
#include <xeq/co_execute.hpp>
#include <xeq/context.hpp>
#include <doctest/doctest.h>

using namespace xeq;

coro<void> trivial() { co_return; }

TEST_CASE("trivial") {
    context ctx;
    co_spawn(ctx, trivial());
    ctx.run();
}

coro<int> five() { co_return 5; }
coro<int> seven() { co_return 7; }

coro<void> twelve(int& result) {
    result = co_await five() + co_await seven();
}

TEST_CASE("spawn") {
    context ctx;
    int result = 0;
    co_spawn(ctx, twelve(result));
    ctx.run();
    CHECK(result == 12);
}

coro<int> maybe_throw(int level, bool t) {
    if (level == 0 && t) {
        throw std::runtime_error("ex");
    }
    if (level == 0) {
        co_return 0;
    }
    co_return 1 + co_await maybe_throw(level - 1, t);
}

TEST_CASE("execute") {
    CHECK(co_execute(five()) == 5);
    CHECK(co_execute(maybe_throw(6, false)) == 6);
    CHECK_THROWS_WITH(co_execute(maybe_throw(3, true)), "ex");
}
