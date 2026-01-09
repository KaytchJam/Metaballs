#pragma once

#include <cstddef>
#include <cassert>
#include <cmath>
#include <cstdint>

#include <type_traits>

namespace mbl {
    //
    namespace common {

        /** Template expression based vector arithmetic namespace. Only defined for Vec3, and for the
         * purpose of removing glm dependency *within* all code referenced by the Metaball engine. Outside
         * of Metaball Engine's dependencies I'll be using glm pretty liberally. I just don't want people
         * using this code to download additional things or fidget with includes if they just want
         * to use the Metaball Engine alone. */
        namespace lalg {

            template <typename T, size_t N, typename Derived>
            struct VecExpression {
                T operator[](size_t i) const {
                    return static_cast<const Derived&>(*this)[i];
                }

                constexpr size_t size() const {
                    return N;
                }

                template <typename OtherDerivation>
                bool operator==(const VecExpression<T,N,OtherDerivation>& other) const {
                    const Derived& v = static_cast<const Derived&>(*this);
                    for (size_t i = 0; i < N; i++) {
                        if (v[i] != other[i]) {
                            return false;
                        }
                    }
                    return true;
                }

                using value_type = T;
                static constexpr bool is_leaf = false;
            };

            template <typename T, size_t N>
            struct Vec : public VecExpression<T,N,Vec<T,N>> {
                using value_type = T;
                T data[N];

                constexpr T& operator[](size_t i) {
                    return data[i];
                }

                constexpr const T& operator[](size_t i) const {
                    return data[i];
                }

                constexpr size_t size() const {
                    return N;
                }

                static constexpr bool is_leaf = true;
            };

            template <typename T>
            struct Vec<T,3> : public VecExpression<T,3,Vec<T,3>> {
                union { T x, r, s; };
                union { T y, g, t; };
                union { T z, b, p; };

                using value_type = T;
                static constexpr bool is_leaf = true;

                constexpr T& operator[](size_t i) {
                    assert(i < 3);
                    switch (i) {
                        default:
                        case 0: return x;
                        case 1: return y;
                        case 2: return z;
                    }
                }

                constexpr const T& operator[](size_t i) const {
                    assert(i < 3);
                    switch (i) {
                        default:
                        case 0: return x;
                        case 1: return y;
                        case 2: return z;
                    }
                }

                constexpr bool operator==(const Vec& other) const {
                    return x == other.x && y == other.y && z == other.x;
                }

                constexpr size_t size() const {
                    return 3;
                }

                // templated constructors

                template <typename Derived> Vec(const VecExpression<T,3,Derived>& ve) : x(ve[0]), y(ve[1]), z(ve[2]) {}
                template <typename U, typename Derived> Vec(const VecExpression<U,3,Derived>& ve) : x((T)ve[0]), y((T)ve[1]), z((T)ve[2]) {}
                template <typename U> constexpr Vec(const Vec<U,3>& other) : x(other.x), y(other.y), z(other.z) {}

                // normal constructors

                constexpr Vec() : x(), y(), z() {}
                constexpr Vec(const T& px, const T& py, const T& pz) : x(px), y(py), z(pz) {}
                constexpr Vec(const T& t) : x(t), y(t), z(t) {}
                constexpr Vec(const Vec& other) : x(other.x), y(other.y), z(other.z) {}
            };

            /** Representation of the sum of 2 vectors */
            template <typename T, size_t N, typename Exp1, typename Exp2>
            class SumVec : public VecExpression<T,N,SumVec<T,N,Exp1,Exp2>> {
            private:
                typename std::conditional<Exp1::is_leaf, const Exp1&, const Exp1>::type v;
                typename std::conditional<Exp2::is_leaf, const Exp2&, const Exp2>::type w;
            public:

                SumVec(const Exp1& e1, const Exp2& e2) : v(e1), w(e2) {
                    static_assert(std::is_same<typename Exp1::value_type, typename Exp2::value_type>::value);
                }

                T operator[](size_t i) const {
                    return v[i] + w[i];
                }

                constexpr size_t size() const {
                    return N;
                }

                using value_type = T;
                static constexpr bool is_leaf = false;
            };

            /** Representation of the difference of 2 vectors */
            template <typename T, size_t N, typename Exp1, typename Exp2>
            class SubVec : public VecExpression<T,N,SubVec<T,N,Exp1,Exp2>> {
            private:
                typename std::conditional<Exp1::is_leaf, const Exp1&, const Exp1>::type v;
                typename std::conditional<Exp2::is_leaf, const Exp2&, const Exp2>::type w;
            public:

                SubVec(const Exp1& e1, const Exp2& e2) : v(v), w(w) {
                    static_assert(std::is_same<typename Exp1::value_type, typename Exp2::value_type>::value);
                }

                T operator[](size_t i) const {
                    return v[i] - w[i];
                }

                constexpr size_t size() const {
                    return N;
                }

                using value_type = T;
                static constexpr bool is_leaf = false;
            };

            /** Representation of multiplying a vector with a scalar */
            template <typename T, size_t N, typename Exp1, typename Scalar>
            class ScaleVec : public VecExpression<T,N,ScaleVec<T,N,Exp1,Scalar>> {
            private:
                typename std::conditional<Exp1::is_leaf, const Exp1&, const Exp1>::type v;
                const Scalar scalar;
            public:
                ScaleVec(const Exp1& v, Scalar scalar) : v(v), scalar(scalar) {}

                T operator[](size_t i) const {
                    return (T) (v[i] * scalar);
                }

                constexpr size_t size() const {
                    return N;
                }

                using value_type = T;
                static constexpr bool is_leaf = false;
            };

            template <typename T, size_t N, typename Exp1, typename Scalar>
            class DivisorVec : public VecExpression<T,N,DivisorVec<T,N,Exp1,Scalar>> {
            private:
                typename std::conditional<Exp1::is_leaf, const Exp1&, const Exp1>::type v;
                const Scalar scalar;
            public:
                DivisorVec(const Exp1& v, Scalar scalar) : v(v), scalar(scalar) {}

                T operator[](size_t i) const {
                    return (T) (v[i] / scalar);
                }

                constexpr size_t size() const {
                    return N;
                }

                using value_type = T;
                static constexpr bool is_leaf = false;
            };
            
            template <typename T, size_t N, typename E1, typename E2>
            SumVec<T,N,E1,E2> operator+(const VecExpression<T,N,E1>& v, const VecExpression<T,N,E2>& w) {
                return SumVec<T,N,E1,E2>(static_cast<const E1&>(v), static_cast<const E2&>(w));
            }

            template <typename T, size_t N, typename E1, typename E2>
            SubVec<T,N,E1,E2> operator-(const VecExpression<T,N,E1>& v, const VecExpression<T,N,E2>& w) {
                return SubVec<T,N,E1,E2>(static_cast<const E1&>(v), static_cast<const E2&>(w));
            }

            template <typename T, size_t N, typename E1>
            ScaleVec<T,N,E1,int32_t> operator-(const VecExpression<T,N,E1>& v) {
                return ScaleVec<T,N,E1,int32_t>(static_cast<const E1&>(v), -1);
            }

            template <typename T, size_t N, typename E1, typename Scalar>
            ScaleVec<T,N,E1,Scalar> operator*(Scalar scalar, const VecExpression<T,N,E1>& v) {
                return ScaleVec<T,N,E1,Scalar>(static_cast<const E1&>(v), scalar);
            }

            template <typename T, size_t N, typename E1, typename Scalar>
            ScaleVec<T,N,E1,Scalar> operator*(const VecExpression<T,N,E1>& v, Scalar scalar) {
                return ScaleVec<T,N,E1,Scalar>(static_cast<const E1&>(v), scalar);
            }

            template <typename T, size_t N, typename E1, typename Scalar>
            DivisorVec<T,N,E1,Scalar> operator/(const VecExpression<T,N,E1>& v, Scalar scalar) {
                return DivisorVec<T,N,E1,Scalar>(static_cast<const E1&>(v), scalar);
            }

            template <typename T, size_t N, typename D1, typename D2, typename BinaryMap>
            Vec<T,N> binary_map(const VecExpression<T,N,D1>& v, const VecExpression<T,N,D2>& w, const BinaryMap& B) {
                Vec<T,N> u;
                for (size_t i = 0; i < N; i++) {
                    u[i] = B(v[i], w[i]);
                }
                return u;
            }

            template <typename T, size_t N, typename D1, typename D2>
            Vec<T,N> max(const VecExpression<T,N,D1>& v, const VecExpression<T,N,D2>& w) {
                return binary_map(v, w, [](T a, T b){ return std::max(a,b);} );
            }

            template <typename T, size_t N, typename D1, typename D2>
            Vec<T,N> min(const VecExpression<T,N,D1>& v, const VecExpression<T,N,D2>& w) {
                return binary_map(v, w, [](T a, T b){ return std::min(a,b);} );
            }

            /** EAGER EXPRESSIONS ("ACTIVATE" ALL THE LAZY EXPRESSIONS FOR A COMPUTATION) */

            template <typename T, size_t N, typename E1, typename E2>
            T dot(const VecExpression<T,N,E1>& v, const VecExpression<T,N,E2>& w) {
                assert(v.size() == w.size());
                T acc = 0.0;
                for (int i = 0; i < v.size(); i++) {
                    acc += v[i] * w[i];
                }
                return acc;
            }

            template <typename T, size_t N, typename E1>
            T magnitude(const VecExpression<T,N,E1>& v) {
                return std::sqrt(dot(v, v));
            }

            template <typename T, size_t N, typename E1, typename E2>
            T cosineOf(const VecExpression<T,N,E1>& v, const VecExpression<T,N,E2>& w) {
                return (dot(v, w) / (magnitude(v) * magnitude(w)));
            }


            /** HYBRID LAZY + EAGER */

            template <typename T, size_t N, typename E1>
            DivisorVec<T,N,Vec<T,N>,T> unit(const VecExpression<T,N,E1>& v) {
                Vec<T,N> v_evaluated = v;
                const T mag = magnitude(v_evaluated);
                return v_evaluated / mag;
            }

            template <typename T, size_t N, typename E1, typename E2>
            ScaleVec<T,N,Vec<T,N>,T> projectOnto(const VecExpression<T,N,E1>& v, const VecExpression<T,N,E2>& onto) {
                Vec<T,N> b_unit = unit(onto);
                return dot(v, b_unit) * b_unit;
            }

            template <typename T, typename E1, typename E2>
            Vec<T,3> cross(const VecExpression<T,3,E1>& v, const VecExpression<T,3,E2>& w) {
                Vec<T,3> i(1.0, 0.0, 0.0);
                Vec<T,3> j(0.0, 1.0, 0.0);
                Vec<T,3> k(0.0, 0.0, 1.0);

                return ((v[1] * w[2] - v[2] * v[1]) * i) 
                - ((v[0] * w[2] - v[2] * w[0]) * j)
                + ((v[0] * w[1] - v[1] * w[0]) * k);
            }

            using Vec3 = Vec<float,3>;
            using IVec3 = Vec<int32_t,3>;
        }
    }
}