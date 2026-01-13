#include <common/lalg.hpp>

#include <string>
#include <sstream>
#include <iostream>

using namespace mbl::common::lalg;

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

TestResult ptr_test() {
    Vec3 v(10, 25, 13);
    Vec3 u(0);
    float* ptr = &v[0];
    u[0] = *ptr;
    u[1] = *(ptr + 1);
    u[2] = *(ptr + 2);
    return {
        v,
        u
    };
}

TestResult unit_vec_test() {
    Vec3 v(8,0,0);
    return {
        Vec3(1,0,0),
        unit(v)
    };
}

TestResult ivec_unit_vec() {
    IVec3 v(10, -12, 7);
    Vec3 expected = Vec3(0.58420624f,-0.70104749f,0.40894437f);
    Vec3 actual = unit(v);

    bool mismatch = false;
    for (size_t i = 0; i < 3; i++) {
        mismatch = !(std::abs(expected[i] - actual[i]) <= 1e-3f);
    }

    if (!mismatch) {
        actual = expected;
    }

    return {
        expected,
        actual
    };
}

TestResult dot_product_test() {
    Vec3 v(0,1,0);
    Vec3 u(1,0,0);
    return {
        Vec3(0),
        Vec3(dot(v,u))
    };
}

TestResult dot_product_two() {
    Vec3 v(11, 2, 8);
    Vec3 u(2, 5, 1);
    return {
        Vec3(40),
        Vec3(dot(v,u))
    };
}

TestResult project_test() {
    Vec3 v(4,5,0);
    Vec3 u(0,1,0);    

    return {
        Vec3(0,5,0),
        project_onto(v,u)
    };
}

TestResult cross_prod_test() {
    Vec3 v(1,0,0);
    Vec3 u(0,1,0);
    return {
        Vec3(0,0,1),
        cross(v,u)
    };
}

TestResult cosine_test() {
    Vec3 v(2,4,3);
    Vec3 u(1,8,5);

    float expected = 0.959126f;
    float actual = cosine_of(v,u);

    if (std::abs(expected - actual) <= 1e-3f) {
        actual = expected;
    }

    return {
        Vec3(expected),
        Vec3(actual)
    };
}

TestResult vec_to_ivec_test() {
    Vec3 v(10.5f, 0.0f, 2.9f);
    Vec3 u(0.6f, 0.9f, 2.05f);
    IVec3 q = v + u;
    return {
        Vec3(11,0,4),
        q
    };
}

TestResult binmax_map_test() {
    Vec3 u(1,11,7);
    Vec3 v(2,3,5);
    Vec3 q = max(u,v);
    return {
        Vec3(2,11,7),
        q
    };
}

TestResult binmin_map_test() {
    Vec3 v(-11, 5, 42);
    IVec3 u(10, -8, 50);
    return {
        Vec3(-11,-8,42),
        min(v,Vec3(u))
    };
}

TestResult elemwise_mul_test() {
    Vec3 v1(10, 11, 2);
    Vec3 v2(0, 2, 5);

    return {
        Vec3(0, 22, 10),
        elementwise(v1, v2)
    };
}

TestResult compose_elementwise_add_magnitude() {
    Vec3 v(10, 5, 2);
    Vec3 u(2);
    Vec3 q(-5, -20, 6);

    return {
        Vec3(15, -10, 10),
        elementwise(v,u) + q
    };
}

struct Ray { Vec3 origin; Vec3 direction; };

TestResult ray_plane_intersection_point_test() {
    // ray origin & direction
    Ray R = { Vec3(0,0,4), unit(Vec3(2,2,-3)) };

    // triangle vertex coordinates
    const Vec3 A = Vec3(4,3,1);
    const Vec3 B = Vec3(2,3,0);
    const Vec3 C = Vec3(1,1,1);

    const Vec3 N = unit(cross(A - B, C - B));
    const float t = dot(N, B - R.origin) / dot(N, R.direction);
    const float expected_t = 4.5354f;

    Vec3 actual = R.origin + R.direction * t;
    const Vec3 expected = Vec3(2.2f,2.2f,0.7f);

    if (std::abs(t - expected_t) <= 1e-3f) {
        actual = expected;
    }

    return {
        expected,
        actual
    };
}

TestResult ray_sphere_intersection_test() {
    const Ray R = { Vec3(0,0,4), unit(Vec3(2,2,-3)) };

    // Circle Center & Radius
    const Vec3 center = Vec3(2.f + (1.0f/3.0f), 2.f + (1.0f/3.0f), 2.0f / 3.0f);
    const float radius = 1.f;
    const float radius_squared = radius * radius;

    // Compute quadratic formula
    const float a = dot(R.direction, R.direction);
    const float b = 2.0f * dot(R.origin, R.direction) - 2.0f * dot(center, R.direction);
    const float c = -2.0f * dot(center, R.origin) + dot(center, center) + dot(R.origin, R.origin) - radius_squared;

    // Check if we have a real solution via discriminant
    const Vec3 expected = Vec3(1.79261938f,1.79261938f,1.31107093f);
    const float discriminant = b*b - 4.0f*a*c;
    if (discriminant < 0) {
        return {
            expected,
            Vec3(std::numeric_limits<float>::max())
        };
    }

    // Real quadratic formula solutions
    const float t1 = (-b - std::sqrt(discriminant)) / (2.0f * a);
    const float t2 = (-b + std::sqrt(discriminant)) / (2.0f * a);
    const float t = t1 >= 0 ? t1 : t2;

    // Get the intersection point
    Vec3 actual = R.origin + R.direction * t;

    // Is it close to our expected solution (by some error 1e-3f)
    bool match = true;
    for (int i = 0; i < 3; i++) { match = match & (std::abs(actual[i] - expected[i]) <= 1e-3f); }
    actual = match ? expected : actual;

    return {
        expected,
        actual
    };
}

TestResult map_pow_test() {
    Vec3 v(2, 4, 8);
    return {
        Vec3(4,16,64),
        map(v, [](float v){ return v * v; })
    };
}

TestResult fold_sum() {
    Vec3 v(1, 2, 3);
    return {
        Vec3(6),
        Vec3(fold(v, 0.f, [](float a, float b){ return a + b; }))
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
        { "Div IP #1", div_in_place_test },
        { "Vec3 Contiguity #1", ptr_test },
        { "Unit/1 Vec #1", unit_vec_test },
        { "DotProd #1", dot_product_test },
        { "Project Vec #1", project_test },
        { "Dot Prod #2", dot_product_two },
        { "Cross Pr #1", cross_prod_test },
        { "Cosine Calc #1" , cosine_test },
        { "IVec3 Unit #1", ivec_unit_vec },
        { "V to IV #1", vec_to_ivec_test },
        { "Mapply #1", binmax_map_test },
        { "Mapply #2", binmin_map_test },
        { "Elementwise #1", elemwise_mul_test },
        { "Multiple Composition #1", compose_elementwise_add_magnitude },
        { "Ray-Plane Intersection #1", ray_plane_intersection_point_test },
        { "Ray-Sphere Interesection #1", ray_sphere_intersection_test },
        { "Map Test #1", map_pow_test },
        { "Fold #1", fold_sum }
    };
    
    std::cout << "LALG TESTS\n========================" << std::endl;
    for (TestItem& t : tests) {
        TestResult res = t.test_func();
        bool equal = res.actual == res.expected;
        std::cout << t.test_name << 
            ": EXPECTED = " << printvec(res.expected) << 
            ", ACTUAL = " << printvec(res.actual) << 
            ", " << ((equal) ? "PASS!" : "FAIL...") << std::endl;
    }
}