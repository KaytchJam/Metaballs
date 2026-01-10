#include <common/lalg.hpp>

#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <array>

using namespace mbl::common::lalg2;

template <typename T, size_t N>
std::string printvec(const Vec<T,N>& v) {
    std::stringstream ss;
    ss << "[";
    if (N > 0) { ss << v[0]; }
    if (N >= 1) { for (int i = 1; i < N; i++) { ss << ", " << v[i]; } }
    ss << "]";
    return ss.str();
}

struct TestResult { Vec3 expected; Vec3 actual; };
typedef TestResult (*TestFunction)();
struct TestItem { const char* test_name; TestFunction test_func; };

TestResult add_test() {
    Vec3 a(1.0f);
    Vec3 b(2.0, 3.0, 5.0);
    return {
        { 3.0, 4.0, 6.0 },
        a + b
    };
}

TestResult sub_test() {
    Vec3 v(1);
    Vec3 u(1.0f);
    return  {
        Vec3(0.f),
        v - u
    };
}

TestResult mul_test() {
    Vec3 v(2, 3, 4);
    float s = 2;
    return {
        Vec3(4, 6, 8),
        v * s
    };
}

TestResult div_test() {
    Vec3 v(10);
    for (int i = 0; i < 2; i++) { v = v / 2; }
    return {
        Vec3(2.5),
        v
    };
}

TestResult add_in_place_test() {
    Vec3 v(5);
    v += Vec3(100);
    return {
        Vec3(105),
        v
    };
}

TestResult sub_in_place_test() {
    Vec3 v(105);
    v -= Vec3(100);
    return {
        Vec3(5),
        v
    };
}

TestResult mul_in_place_test() {
    Vec3 v(50);
    return {
        Vec3(100),
        v * 2
    };
}

TestResult div_in_place_test() {
    Vec3 v(100);
    return {
        Vec3(50),
        v / 2
    };
}

int main() {
    TestItem tests[] = { 
        { "Add #1", add_test },
        { "Sub #1", sub_test },
        { "Mul #1", mul_test },
        { "Div #1", div_test },
        { "Add IP #1", add_in_place_test },
        { "Sub IP #1", sub_in_place_test },
        { "Mul IP #1", mul_in_place_test },
        { "Div IP #1", div_in_place_test }
    };

    std::cout << "LALG TESTS\n========================" << std::endl;
    for (TestItem& t : tests) {
        TestResult res = t.test_func();
        bool equal = res.actual == res.expected;
        std::cout << t.test_name << 
            ": EXPECTED = " << printvec(res.expected) << 
            ", ACTUAL = " << printvec(res.actual) << 
            ", " << ((equal) ? "PASS" : "FAIL") << std::endl;
    }
}