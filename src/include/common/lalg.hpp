#pragma once

#include <cstddef>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <utility>
#include <functional>

#include <type_traits>

namespace mbl {
    //
    namespace common {

        /** Template expression based vector arithmetic namespace. Currenlty only defined for Vec3, and for the
         * purpose of removing glm dependency *within* all code referenced by the Metaball engine. Outside
         * of Metaball Engine's dependencies I'll be using glm pretty liberally. I just don't want people
         * using this code to download additional things or fidget with includes if they just want
         * to use the Metaball Engine alone. */
        namespace lalg {
            template<typename V>
            struct VecTraits;

            template<typename V>
            using VecValue = typename VecTraits<V>::value_type;

            template<typename V>
            constexpr size_t VecSize = VecTraits<V>::size;

            template<typename T>
            using HasSubscript = decltype(std::declval<const T&>()[0]);

            template <typename F, typename S1, typename S2>
            using OpResult = std::invoke_result_t<F, S1, S2>;

            template <typename F, typename S>
            using UnaryOpResult = std::invoke_result_t<F,S>;

            template<typename V>
            concept VecLike =
                requires {
                    typename VecTraits<V>::value_type;
                    { VecTraits<V>::size } -> std::convertible_to<size_t>;
                } && 
                requires (const V& v) { v[0]; 
            };

            /** Simple representation of a Algebraic Vector over some type `T` with dimension `N`. */
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

                template<VecLike Expr>
                Vec(const Expr& e) : x((T)e[0]), y((T)e[1]), z((T)e[2]) {}

                template <VecLike Expr>
                Vec& operator+=(const Expr& rhs) {
                    static_assert(VecSize<Expr> == 3);
                    x += rhs[0];
                    y += rhs[1];
                    z += rhs[2];
                    return *this;
                }

                template <VecLike Expr>
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
            
            /** Vector expression that sums two VecLikes. */
            template<typename L, typename R>
            struct SumVec {
                const L& a;
                const R& b;
                
                using value_type = OpResult<std::plus<>, VecValue<L>, VecValue<R>>;
                static constexpr size_t size = VecSize<L>;

                value_type operator[](size_t i) const { return a[i] + b[i]; }
            };
            
            /** Vector expression that subtracts two VecLikes. */
            template<typename L, typename R>
            struct SubVec {
                const L& a;
                const R& b;

                using value_type = OpResult<std::minus<>, VecValue<L>, VecValue<R>>;
                static constexpr size_t size = VecSize<L>;
                
                value_type operator[](size_t i) const { return a[i] - b[i]; }
            };
            
            /** Vector expression that scales a VecLike */
            template<typename V, typename S>
            struct ScaleVec {
                const V& v;
                S s;

                using value_type = OpResult<std::multiplies<>, VecValue<V>, S>;
                static constexpr size_t size = VecSize<V>;
                
                value_type operator[](size_t i) const { return v[i] * s; }
            };
            
            /** Vector expression that divides a VecLike */
            template<typename V, typename S>
            struct DivVec {
                const V& v;
                S s;

                using value_type = OpResult<std::divides<>, VecValue<V>, S>;
                static constexpr size_t size = VecSize<V>;

                value_type operator[](size_t i) const { return v[i] / s; }
            };

            /** Vector expression that combines two VecLikes via some BinaryMap function. */
            template<typename L, typename R, typename M>
            struct BinaryMapVec {
                const L& a;
                const R& b;
                const M& m;

                using value_type = OpResult<M, VecValue<L>, VecValue<R>>;
                static constexpr size_t size = VecSize<L>;
                
                value_type operator[](size_t i) const { return m(a[i],b[i]); }
            };

            /** Vector expression that maps a function over some VecLike. */
            template<typename V, typename M>
            struct MapVec {
                const V& v;
                const M& m;

                using value_type = UnaryOpResult<M,VecValue<V>>;
                static constexpr size_t size = VecSize<V>;
                
                value_type operator[](size_t i) const { return m(v[i]); }
            };

            template<typename L, typename R>
            struct VecTraits<SumVec<L,R>> {
                using value_type = OpResult<std::plus<>, VecValue<L>, VecValue<R>>;
                static constexpr size_t size = VecSize<L>;
            };

            template<typename L, typename R>
            struct VecTraits<SubVec<L,R>> {
                using value_type = OpResult<std::minus<>, VecValue<L>, VecValue<R>>;
                static constexpr size_t size = VecSize<L>;
            };

            template<typename V, typename S>
            struct VecTraits<ScaleVec<V,S>> {
                using value_type = OpResult<std::multiplies<>, VecValue<V>, S>;
                static constexpr size_t size = VecSize<V>;
            };

            template<typename V, typename S>
            struct VecTraits<DivVec<V,S>> {
                using value_type = OpResult<std::divides<>, VecValue<V>, S>;
                static constexpr size_t size = VecSize<V>;
            };

            template<typename L, typename R, typename M>
            struct VecTraits<BinaryMapVec<L,R,M>> {
                using value_type = OpResult<M, VecValue<L>, VecValue<R>>;
                static constexpr size_t size = VecSize<L>;
            };

            template<typename V, typename M>
            struct VecTraits<MapVec<V,M>> {
                using value_type = UnaryOpResult<M,VecValue<V>>;
                static constexpr size_t size = VecSize<V>;
            };

            template<typename L, typename R>
            SumVec<L,R> operator+(const L& l, const R& r) {
                static_assert(VecSize<L> == VecSize<R>, "lalg::operator+(Vec<L> lhs, Vec<R> rhs): lhs and rhs must have the same size.");
                return SumVec<L,R>{l,r};
            }
            
            template<typename L, typename R>
            SubVec<L,R> operator-(const L& l, const R& r) {
                static_assert(VecSize<L> == VecSize<R>, "lalg::operator-(Vec<L> lhs, Vec<R> rhs): lhs and rhs must have the same size.");
                return SubVec<L,R>{l,r};
            }

            template<VecLike V, typename S>
            ScaleVec<V,S> operator*(const V& v, S s) {
                return ScaleVec<V,S>{v,s};
            }

            template<VecLike V, typename S>
            ScaleVec<V,S> operator*(S s, const V& v) {
                return ScaleVec<V,S>{v,s};
            }

            template<VecLike V, typename S>
            DivVec<V,S> operator/(const V& v, S s) {
                return DivVec<V,S>{v,s};
            }
            
            template <VecLike L, VecLike R, typename BinaryMap> requires std::is_invocable_v<BinaryMap,VecValue<L>,VecValue<R>>
            BinaryMapVec<L,R,BinaryMap> mapply(const L& l, const R& r, const BinaryMap& b) {
                static_assert(VecSize<L> == VecSize<R>, "lalg::mapply(Vec<L> lhs, Vec<R> rhs): lhs and rhs must have the same size.");
                return BinaryMapVec<L,R,BinaryMap>{l,r,b};
            }

            template <VecLike L, VecLike R>
            bool operator==(const L& l, const R& r) {
                static_assert(VecSize<L> == VecSize<R>, "lalg::operator==(Vec<L> lhs, Vec<R> rhs): lhs and rhs must have the same size.");
                for (int i = 0; i < VecSize<L>; i++) {
                    if (l[i] != r[i]) {
                        return false;
                    }
                }
                return true;
            }
            
            template <VecLike L, VecLike R>
            bool operator!=(const L& l, const R& r) {
                static_assert(VecSize<L> == VecSize<R>, "lalg::operator!=(Vec<L> lhs, Vec<R> rhs): lhs and rhs must have the same size.");
                return !(l == r);
            }

            /** Apply some function `Map m` over the elements in `VecLike v`. */
            template <VecLike V, typename Map> requires std::is_invocable_v<Map, VecValue<V>>
            MapVec<V,Map> map(const V& v, const Map& m) {
                return MapVec<V,Map>{v,m};
            }

            /** Return the dot product between `VecLike v` and `VecLike w`  */
            template <VecLike L, VecLike R> 
            VecValue<L> dot(const L& v, const R& w) {
                static_assert(VecSize<L> == VecSize<R>, "lalg::cosine_of(Vec<L> lhs, Vec<R> rhs): lhs and rhs must have the same size.");
                static_assert(std::is_same<VecValue<L>, VecValue<R>>::value, "lalg::dot(Vec<L> lhs, Vec<R> rhs): lhs and rhs must be vectors of the same underlying type.");

                using T = VecValue<L>;
                T acc = {};
                for (int i = 0; i < VecSize<L>; i++) {
                    acc += v[i] * w[i];
                }
                return acc;
            }

            /** Return the magnitude of `VecLike v` */
            template <VecLike V>
            float magnitude(const V& v) {
                return std::sqrt((float) dot(v, v));
            }

            /** Return the cosine of `VecLike v` and `VecLike w`. */
            template <VecLike L, VecLike R>
            float cosine_of(const L& v, const R& w) {
                static_assert(VecSize<L> == VecSize<R>, "lalg::cosine_of(Vec<L> lhs, Vec<R> rhs): lhs and rhs must have the same size.");
                static_assert(std::is_same<VecValue<L>, VecValue<R>>::value, "lalg::cosine_of(Vec<L> lhs, Vec<R> rhs): lhs and rhs must be vectors of the same underlying type.");

                float dacc = {};
                float vacc = {};
                float wacc = {};

                for (size_t i = 0; i < VecSize<L>; i++) {
                    const float vur = v[i];
                    const float wur = w[i];
                    
                    vacc += vur * vur;
                    wacc += wur * wur;
                    dacc += vur * wur;
                }

                return dacc / (std::sqrt(vacc) * std::sqrt(wacc));
            }

            /** HYBRID LAZY + EAGER */

            /** Returns a `VecLike` that is the input `VecLike v` but normalized. */
            template <VecLike V>
            DivVec<V,float> unit(const V& v) {
                return v / magnitude(v);
            }

            /** Produce a new `VecLike` that is the project of `VecLike v` onto `VecLike onto`. */
            template <VecLike L, VecLike R>
            Vec<VecValue<L>,VecSize<L>> project_onto(const L& v, const R& onto) {
                static_assert(std::is_same<VecValue<L>, VecValue<R>>::value, "lalg::project_onto(Vec<L> lhs, Vec<R> rhs): lhs and rhs must be vectors of the same underlying type.");
                Vec<float,VecSize<L>> b_unit = unit(onto);
                return dot(v, b_unit) * b_unit;
            }

            /** Produce a new `VecLike` that is the cross product of `l` and `r`. */
            template <VecLike L, VecLike R>
            Vec<OpResult<std::multiplies<>, VecValue<L>, VecValue<R>>,3> cross(const L& v, const R& w) {
                using T = OpResult<std::multiplies<>, VecValue<L>, VecValue<R>>;
                Vec<T,3> i(1.0, 0.0, 0.0);
                Vec<T,3> j(0.0, 1.0, 0.0);
                Vec<T,3> k(0.0, 0.0, 1.0);

                return ((v[1] * w[2] - v[2] * w[1]) * i) 
                - ((v[0] * w[2] - v[2] * w[0]) * j)
                + ((v[0] * w[1] - v[1] * w[0]) * k);
            }

            /** Produce a new `VecLike` where each element i is the maximum of `l[i]` and `r[i]`. */
            template <VecLike L, VecLike R>
            auto max(const L& l, const R& r) {
                static_assert(std::is_same<VecValue<L>, VecValue<R>>::value, "lalg::max(Vec<L> lhs, Vec<R> rhs): lhs and rhs must be vectors of the same underlying type.");

                using T = VecValue<L>;
                return mapply(l, r, [](const T& a, const T& b) -> T { return std::max(a,b); });
            }

            /** Produce a new `VecLike` where each element i is the minimum of `l[i]` and `r[i]`. */
            template <VecLike L, VecLike R>
            auto min(const L& l, const R& r) {
                static_assert(std::is_same<VecValue<L>, VecValue<R>>::value, "lalg::min(Vec<L> lhs, Vec<R> rhs): lhs and rhs must be vectors of the same underlying type.");

                using T = VecValue<L>;
                return mapply(l, r, [](const T& a, const T& b) -> T { return std::min(a,b); });
            }

            /** Produce a new `VecLike` that is the elementwise product of input `VecLike L` and `VecLike R`. */
            template <VecLike L, VecLike R>
            auto elementwise(const L& l, const R& r) {
                static_assert(std::is_same<VecValue<L>, VecValue<R>>::value, "lalg::min(Vec<L> lhs, Vec<R> rhs): lhs and rhs must be vectors of the same underlying type.");

                using T1 = VecValue<L>;
                using T2 = VecValue<R>;
                return mapply(l, r, [](const T1& a, const T2& b){ return a * b; });
            }

            template <VecLike V, typename Fold> requires std::is_invocable_v<Fold,VecValue<V>,VecValue<V>>
            VecValue<V> fold(const V& v, VecValue<V> accumulator, const Fold& f) {
                for (size_t i = 0; i < VecSize<V>; i++) {
                    accumulator = f(accumulator, v[i]);
                }
                return accumulator;
            }

            /** Compute the squared distance between two `VecLike`s. */
            template <VecLike L, VecLike R> requires std::is_default_constructible_v<VecValue<L>> && std::is_same_v<VecValue<L>,VecValue<R>>
            VecValue<L> distance_squared(const L& l, const R& r) {
                using T = VecValue<L>;
                T acc = {};
                for (size_t i = 0; i < VecSize<L>; i++) {
                    T diff = l[i] - r[i];
                    acc = acc + (diff * diff);
                }
                return acc;
            }

            /** Compute the distance between two `VecLike`s. */
            template <VecLike L, VecLike R> requires std::is_default_constructible_v<VecValue<L>> && std::is_same_v<VecValue<L>,VecValue<R>>
            float distance(const L& l, const R& r) {
                return std::sqrtf((float) distance_squared(l,r));
            }

            /** Clamp all values in some `VecLike` between low and high thresholds. */
            template <VecLike V>
            auto clamp(const V& v, VecValue<V> low, VecValue<V> high) {
                return map(v, [low,high](VecValue<V> i){
                    if (i < low) return low;
                    if (i > high) return high;
                    return i;
                });
            }

            /** Check whether VecLike  */
            template <VecLike L, VecLike R> requires std::is_same_v<VecValue<L>,VecValue<R>> && std::is_floating_point_v<VecValue<L>>
            bool equal_eps(const L& l, const R& r, VecValue<L> eps = 1e-3f) {
                static_assert(VecSize<L> == VecSize<R>, "lalg::cosine_of(Vec<L> lhs, Vec<R> rhs): lhs and rhs must have the same size.");

                for (size_t i = 0; i < VecSize<L>; i++) {
                    if (std::abs(l[i] - r[i]) > eps) {
                        return false;
                    }
                }
                return true;
            }

            using Vec3 = Vec<float,3>;
            using IVec3 = Vec<int32_t,3>;
        }
    }
}