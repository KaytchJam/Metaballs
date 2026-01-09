#pragma once

#include "../dependencies/glm/glm.hpp"

#include <functional>
#include <metaball_traits.hpp>
#include <typeinfo>

namespace mbl {
    /** A run-time interface for Metaballs */ 
    class DynamicMetaball {
    public:
        DynamicMetaball() {}
        ~DynamicMetaball() {}
        virtual float operator()(float x, float y, float z) const = 0;
        virtual float compute(const glm::vec3& v) const {
            return (*this)(v.x, v.y, v.z);
        }
    };

    /** A compile-time interface for Metaballs */
    template <typename Derived>
    class MetaballExpression : public DynamicMetaball {
    public:
        float operator()(float x, float y, float z) const {
            return static_cast<const Derived&>(*this)(x, y, z);
        }
    
        float compute(const glm::vec3& v) const {
            return (*this)(v.x, v.y, v.z);
        }
    };
    
    /** 
     * Wrapper around a 3D Scalar Function. Can be added with other 
     * `MetaballExpressions`. Converting a Metaball Expression
     * back into a Metaball requires the use of the `make_metaball`
     * static function. See the example below.
     * 
     * @code
     * Metaball m1 = Metaball([](float x, float y, float z){ return x + y + z; });
     * Metaball<InverseSquareBlob> m2;
     * Metaball m3 = make_metaball(m1 + m2);
     * @endcode
     * */
    template <typename T, typename IsBounded = void>
    class Metaball : public MetaballExpression<Metaball<T>> {
    private:
        T m_scalar_func;
    public:
        using Inner = T;
        explicit Metaball(T p_scalar_func) : m_scalar_func(std::move(p_scalar_func)) {}
    
        float operator()(float x, float y, float z) const {
            return m_scalar_func(x, y, z);
        }
    
        /** Expose the inner `T` powering this Metaball */
        T& unwrap() { return m_scalar_func; }
        const T& unwrap() const { return m_scalar_func;}
    };

    template <typename T>
    class Metaball<T, std::enable_if_t<HasBoundingBox<T>::value>> : public MetaballExpression<Metaball<T>> {
    private:
        T m_scalar_func;
    public:
        using Inner = T;
        explicit Metaball(T p_scalar_func) : m_scalar_func(std::move(p_scalar_func)) {}
    
        float operator()(float x, float y, float z) const {
            return m_scalar_func(x, y, z);
        }
    
        /** Expose the inner `T` powering this Metaball */
        T& unwrap() { return m_scalar_func; }
        const T& unwrap() const { return m_scalar_func;}
        BoundingBox get_bounding_box() const { return m_scalar_func.get_bounding_box(); }
    };

    template <typename Derived>
    static auto make_metaball(const MetaballExpression<Derived>& me_expr) {
        return Metaball([expr_copy = static_cast<const Derived&>(me_expr)](float x, float y, float z) {
            return expr_copy(x, y, z);
        });
    }
    
    template <typename LHS, typename RHS>
    class MetaballSum : public MetaballExpression<MetaballSum<LHS,RHS>> {
    private:
        const LHS& L;
        const RHS& R;
    public:
        MetaballSum(const LHS& l, const RHS& r) : L(l), R(r) {}
    
        float operator()(float x, float y, float z) const {
            return L(x,y,z) + R(x,y,z);
        }
    };
    
    template <typename LHS, typename RHS>
    MetaballSum<LHS, RHS> operator+(const MetaballExpression<LHS>& lhs, const MetaballExpression<RHS>& rhs) {
        return MetaballSum<LHS, RHS>(
            static_cast<const LHS&>(lhs),
            static_cast<const RHS&>(rhs)
        );
    }
    
    /** Metaball that uses type erasure to aggregate other Metaball types. Though
     * it's "Heavier" than other metaball types, it can consume vector expressions 
     * without undergoing a type change, unlike the normal Metaball<>. */
    class AggregateMetaball : public MetaballExpression<AggregateMetaball> {
    private:
        std::function<float(float,float,float)> m_scalar_func;
    public:
        template <typename T>
        explicit AggregateMetaball(T func) : m_scalar_func(std::move(func)) {
            static_assert(mbl::IsScalarFunction<T>::value, "mbl::AggregateMetaball, Type T does not implement operator()(float x, float y, float z) const -> float");
        }
    
        template <typename Derived>
        AggregateMetaball(const MetaballExpression<Derived>& expr)
            : m_scalar_func([copied_func = static_cast<const Derived&>(expr)](float x, float y, float z) {
                return copied_func(x, y, z);
            }) {}
        
        float operator()(float x, float y, float z) const {
            return m_scalar_func(x, y, z);
        }

        template <typename T>
        T* target() {
            return m_scalar_func.target<T>();
        }

        const std::type_info& target_info() {
            return m_scalar_func.target_type();
        }
    };
    
    template <typename F>
    Metaball(F&&) -> Metaball<std::decay_t<F>>;  // deduction guide
}
