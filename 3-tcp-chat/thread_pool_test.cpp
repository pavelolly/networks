#include "thread_pool.hpp"

struct Foo {
    Foo(int a) : a(a) {
        Println("Foo(int) called ({})", a);
    }
    Foo(const Foo& f) {
        Println("Foo(const Foo &) called ({})", f.a);
        a = f.a;
    };
    Foo(Foo &&f) {
        Println("Foo(Foo &&) called ({})", f.a);
        a = f.a;
        f.a = 0;
    }
    int a;
};

int main() {
    ThreadPool tp(10);
    tp.SetLoggingFlag(false);

    auto l = [](Foo &a, Foo b, Foo c){ Println("Hello 1 a={}, b={} c={}", a.a, b.a, c.a); a.a = 4; };
    Foo a = 1;
    const Foo b = 2;
    Foo c = 3;
    tp.NewTask(l, std::ref(a), b, std::move(c));

    std::this_thread::sleep_for(std::chrono::seconds(2));
    Println("{}", a.a);

    return 0;
}