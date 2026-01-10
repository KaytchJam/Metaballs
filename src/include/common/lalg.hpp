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
                const Derived& to_derived() const {
                    return static_cast<const Derived&>(*this);
                }

                T operator[](size_t i) const {
                    return to_derived()[i];
                }

                constexpr size_t size() const {
                    return N;
                }

                template <typename OtherDerivation>
                bool operator==(const VecExpression<T,N,OtherDerivation>& other) const {
                    const Derived& v = to_derived();
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

                SubVec(const Exp1& e1, const Exp2& e2) : v(e1), w(e2) {
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
                return SumVec<T,N,E1,E2>(v.to_derived(), w.to_derived());
            }

            template <typename T, size_t N, typename E1, typename E2>
            SubVec<T,N,E1,E2> operator-(const VecExpression<T,N,E1>& v, const VecExpression<T,N,E2>& w) {
                return SubVec<T,N,E1,E2>(v.to_derived(), w.to_derived());
            }

            template <typename T, size_t N, typename E1>
            ScaleVec<T,N,E1,int32_t> operator-(const VecExpression<T,N,E1>& v) {
                return ScaleVec<T,N,E1,int32_t>(v.to_derived(), -1);
            }

            template <typename T, size_t N, typename E1, typename Scalar>
            ScaleVec<T,N,E1,Scalar> operator*(Scalar scalar, const VecExpression<T,N,E1>& v) {
                return ScaleVec<T,N,E1,Scalar>(v.to_derived(), scalar);
            }

            template <typename T, size_t N, typename E1, typename Scalar>
            ScaleVec<T,N,E1,Scalar> operator*(const VecExpression<T,N,E1>& v, Scalar scalar) {
                return ScaleVec<T,N,E1,Scalar>(v.to_derived(), scalar);
            }

            template <typename T, size_t N, typename E1, typename Scalar>
            DivisorVec<T,N,E1,Scalar> operator/(const VecExpression<T,N,E1>& v, Scalar scalar) {
                return DivisorVec<T,N,E1,Scalar>(v.to_derived(), scalar);
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
            T cosine_of(const VecExpression<T,N,E1>& v, const VecExpression<T,N,E2>& w) {
                return (dot(v, w) / (magnitude(v) * magnitude(w)));
            }

            /** HYBRID LAZY + EAGER */

            template <typename T, size_t N, typename E1>
            DivisorVec<T,N,Vec<T,N>,T> unit(const VecExpression<T,N,E1>& v) {
                return v / magnitude(v_evaluated);
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

        namespace lalg2 {
            template<typename V>
            struct VecTraits;

            template<typename V>
            using VecValue = typename VecTraits<V>::value_type;

            template<typename V>
            constexpr size_t VecSize = VecTraits<V>::size;

            template<typename T>
            using HasSubscript = decltype(std::declval<const T&>()[0]);

            template <typename T, size_t N>
            struct Vec {
                T data[N];

                using value_type = T;
                static constexpr size_t size = N;

                constexpr T& operator[](size_t i)       { return data[i]; }
                constexpr const T& operator[](size_t i) const { return data[i]; }

                Vec() = default;

                template<typename Expr>
                Vec(const Expr& e) {
                    for (size_t i=0; i< N; i++) {
                        data[i] = e[i];
                    }
                }

                // template<typename Expr>
                // Vec& operator=(const Expr& e) {
                //     static_assert(size_v<Expr> == N, "Vector size mismatch");
                //     for (size_t i = 0; i < N; i++) {
                //         data[i] = e[i];
                //     }
                //     return *this;
                // }
            };

            template <typename T>
            struct Vec<T,3> {
                union {
                    T data[3];
                    struct { T x, y, z; };
                };

                using value_type = T;
                static constexpr size_t size = 3;

                constexpr T& operator[](size_t i)       { return data[i]; }
                constexpr const T& operator[](size_t i) const { return data[i]; }

                Vec() : x(),y(),z() {}
                Vec(T a,T b,T c) : x(a),y(b),z(c) {}
                Vec(const T& t) : x(t), y(t), z(t) {}

                template<typename Expr, typename = HasSubscript<Expr>>
                Vec(const Expr& e) : x((T)e[0]), y((T)e[1]), z((T)e[2]) {}

                template <typename Expr, typename = HasSubscript<Expr>>
                Vec& operator+=(const Expr& rhs) {
                    static_assert(VecSize<Expr> == 3);
                    x += rhs[0];
                    y += rhs[1];
                    z += rhs[2];
                    return *this;
                }

                template <typename Expr, typename = HasSubscript<Expr>>
                Vec& operator-=(const Expr& rhs) {
                    static_assert(VecSize<Expr> == 3);
                    x -= rhs[0];
                    y -= rhs[1];
                    z -= rhs[2];
                    return *this;
                }

                Vec& operator*=(const T scalar) {
                    x *= scalar;
                    y *= scalar;
                    z *= scalar;
                    return *this;
                }

                Vec& operator/=(const T scalar) {
                    x /= scalar;
                    y /= scalar;
                    z /= scalar;
                    return *this;
                }
            };

            template<typename T,size_t N>
            struct VecTraits<Vec<T,N>> {
                using value_type = T;
                static constexpr size_t size = N;
            };
            
            template<typename L, typename R>
            struct SumVec {
                const L& a;
                const R& b;
                
                using value_type = VecValue<L>;
                static constexpr size_t size = VecSize<L>;

                value_type operator[](size_t i) const { return a[i] + b[i]; }
            };
            
            template<typename L, typename R>
            struct SubVec {
                const L& a;
                const R& b;

                using value_type = VecValue<L>;
                static constexpr size_t size = VecSize<L>;
                
                value_type operator[](size_t i) const { return a[i] - b[i]; }
            };
            
            template<typename V, typename S>
            struct ScaleVec {
                const V& v;
                S s;

                using value_type = VecValue<V>;
                static constexpr size_t size = VecSize<V>;
                
                value_type operator[](size_t i) const { return v[i] * s; }
            };
            
            template<typename V, typename S>
            struct DivVec {
                const V& v;
                S s;

                using value_type = VecValue<V>;
                static constexpr size_t size = VecSize<V>;

                value_type operator[](size_t i) const { return v[i] / s; }
            };

            template<typename L, typename R>
            struct VecTraits<SumVec<L,R>> {
                using value_type = VecValue<L>;
                static constexpr size_t size = VecSize<L>;
            };

            template<typename L, typename R>
            struct VecTraits<SubVec<L,R>> {
                using value_type = VecValue<L>;
                static constexpr size_t size = VecSize<L>;
            };

            template<typename V, typename S>
            struct VecTraits<ScaleVec<V,S>> {
                using value_type = VecValue<V>;
                static constexpr size_t size = VecSize<V>;
            };

            template<typename V, typename S>
            struct VecTraits<DivVec<V,S>> {
                using value_type = VecValue<V>;
                static constexpr size_t size = VecSize<V>;
            };


            template<typename L, typename R>
            SumVec<L,R> operator+(const L& l, const R& r) {
                static_assert(VecSize<L> == VecSize<R>);
                return SumVec<L,R>{l,r};
            }
            
            template<typename L, typename R>
            SubVec<L,R> operator-(const L& l, const R& r) {
                static_assert(VecSize<L> == VecSize<R>);
                return SubVec<L,R>{l,r};
            }

            template<typename V, typename S, typename = HasSubscript<V>>
            ScaleVec<V,S> operator*(const V& v, S s) {
                return ScaleVec<V,S>{v,s};
            }

            template<typename V, typename S, typename = HasSubscript<V>>
            ScaleVec<V,S> operator*(S s, const V& v) {
                return ScaleVec<V,S>{v,s};
            }

            template<typename V, typename S>
            DivVec<V,S> operator/(const V& v, S s) {
                return DivVec<V,S>{v,s};
            }

            template <typename L, typename R>
            bool operator==(const L& l, const R& r) {
                static_assert(VecSize<L> == VecSize<R>);
                for (int i = 0; i < VecSize<L>; i++) {
                    if (l[i] != r[i]) {
                        return false;
                    }
                }

                return true;
            }

            template <typename L, typename R>
            bool operator!=(const L& l, const R& r) {
                return !(l == r);
            }

            template <typename L, typename R>
            VecValue<L> dot(const L& v, const R& w) {
                static_assert(VecSize<L> == VecSize<R>);
                using T = VecValue<L>;
                T acc = {};
                for (int i = 0; i < VecSize<L>; i++) {
                    acc += v[i] * w[i];
                }
                return acc;
            }

            template <typename V>
            float magnitude(const V& v) {
                return std::sqrt((float) dot(v, v));
            }

            template <typename L, typename R>
            float cosine_of(const L& v, const R& w) {
                return (dot(v, w) / (magnitude(v) * magnitude(w)));
            }

            /** HYBRID LAZY + EAGER */

            template <typename V>
            Vec<float, VecSize<V>> unit(const V& v) {
                return v / magnitude(v);
            }

            template <typename L, typename R>
            ScaleVec<L, float> projectOnto(const L& v, const R& onto) {
                Vec<T,N> b_unit = unit(onto);
                return dot(v, b_unit) * b_unit;
            }

            template <typename L, typename R>
            Vec<VecValue<L>,3> cross(const L& v, const R& w) {
                using T = VecValue<L>;
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