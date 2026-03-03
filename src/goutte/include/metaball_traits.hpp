#pragma once

#include <type_traits>
#include <boundingbox.hpp>

namespace mbl {
    /** 
     * Requirements for being a ScalarFunction
     * (1) have the following operator overload: 
     *      `float operator()(float,float,float) const;`
     * 
     * (2) That's all! */
    template <typename T>
    struct IsScalarFunction
        : std::conjunction<
              std::is_invocable<const T, float, float, float>,
              std::is_same<std::invoke_result_t<const T, float, float, float>, float>
          > {};
    
    template <typename, typename = std::void_t<>>
    struct HasBoundingBox : std::false_type {};

    /** 
     * Requirements for HasBoundingBox:
     * 
     * (1) Have the following function:
     *      `BoundingBox get_bounding_box() const;`
     * 
     * (2) That is all.
     */
    template <typename T>
    struct HasBoundingBox<T, std::void_t<decltype(&T::get_bounding_box)>> 
        : std::is_same<decltype(std::declval<const T>().get_bounding_box()), BoundingBox> {};


    /** 
     * Requirements for being a BoundedScalarFunction
     * 
     * (1) Satisfy `IsScalarFunction<T>` (try `static_assert(IsScalarFunction<YourType>::value)` to check)
     * (2) Satisfy `HasBoundingBox<T>` (try `static_assert(HasBoundingBox<YourType>::value)` to check)
     */
    template <typename PBSF>
    struct IsBoundedScalarFunction 
        : std::conjunction<
                IsScalarFunction<PBSF>,
                HasBoundingBox<PBSF>
        > {};
}
