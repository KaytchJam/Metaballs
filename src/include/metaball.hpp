#pragma once

#include "../dependencies/glm/glm.hpp"

#include <functional>

template <typename Derived>
class MetaballExpression {
public:
    float operator()(float x, float y, float z) const {
        return static_cast<const Derived&>(*this)(x, y, z);
    }

    float compute(const glm::vec3& v) const {
        return (*this)(v.x, v.y, v.z);
    }
};

template <typename ScalarValuedFunction>
class Metaball : public MetaballExpression<Metaball<ScalarValuedFunction>> {
private:
    ScalarValuedFunction m_scalar_func;
public:
    explicit Metaball(ScalarValuedFunction p_scalar_func) : m_scalar_func(std::move(p_scalar_func)) {}

    float operator()(float x, float y, float z) const {
        return m_scalar_func(x, y, z);
    }

    /** Expose the inner `ScalarValuedFunction` powering this Metaball */
    ScalarValuedFunction& unwrap() { return m_scalar_func; }
    const ScalarValuedFunction& unwrap() const { return m_scalar_func;}
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
 * with undergoing a type change, unlike the normal Metaball<>. */
class AggregateMetaball : public MetaballExpression<AggregateMetaball> {
private:
    std::function<float(float,float,float)> m_scalar_func;
public:
    template <typename ScalarValuedFunction>
    explicit AggregateMetaball(ScalarValuedFunction func) : m_scalar_func(std::move(func)) {}

    template <typename Derived>
    AggregateMetaball(const MetaballExpression<Derived>& expr)
        : m_scalar_func([copied_func = static_cast<const Derived&>(expr)](float x, float y, float z) {
            return copied_func(x, y, z);
        }) {}
    
    float operator()(float x, float y, float z) const {
        return m_scalar_func(x, y, z);
    }
};

template <typename F>
Metaball(F&&) -> Metaball<std::decay_t<F>>;  // deduction guide