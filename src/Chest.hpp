#pragma once

#include <variant>
#include <string>
#include <functional>
#include <iostream>

// Exception thrown when Chest is opened but has no stuff
struct NoStuffException : public std::exception {
    std::string msg;
    explicit NoStuffException(std::string m) : msg(std::move(m)) {}
    const char* what() const noexcept override { return msg.c_str(); }
};

// Exception thrown when sign is opened but Chest has stuff
struct HasStuffException : public std::exception {
    const char* what() const noexcept override { return "The chest has stuff."; }
};

template <typename M, typename T>
struct MapResultT {
    using Type = decltype(std::declval<M>()(std::declval<T>()));
};

template <typename M, typename T>
using MapResult = typename MapResultT<M,T>::Type;


// A Chest either holds stuff (T) or a sign (std::string)
template<typename T>
class Chest {
private:
    std::variant<T, std::string> content;

    explicit Chest(T&& item) : content(std::move(item)) {}
    explicit Chest(const T& item) : content(item) {}
    explicit Chest(std::string&& msg) : content(std::move(msg)) {}
    explicit Chest(const std::string& msg) : content(msg) {}

public:
    // Static constructors
    static Chest<T> stuff(const T& item) { return Chest<T>(item); }
    static Chest<T> stuff(T&& item) { return Chest<T>(std::move(item)); }
    static Chest<T> sign(const std::string& msg) { return Chest<T>(msg); }
    static Chest<T> sign(std::string&& msg) { return Chest<T>(std::move(msg)); }

    // Queries
    bool is_stuffed() const { return std::holds_alternative<T>(content); }
    bool is_signed() const { return std::holds_alternative<std::string>(content); }

    template<typename Pred>
    bool is_stuffed_and(Pred pred) const {
        return is_stuffed() && pred(std::get<T>(content));
    }

    template<typename Pred>
    bool is_signed_and(Pred pred) const {
        return is_signed() && pred(std::get<std::string>(content));
    }

    // access
    T& open() {
        if (!is_stuffed()) throw NoStuffException(std::get<std::string>(content));
        return std::get<T>(content);
    }

    const T& open() const {
        if (!is_stuffed()) throw NoStuffException(std::get<std::string>(content));
        return std::get<T>(content);
    }

    std::string& open_sign() {
        if (!is_signed()) throw HasStuffException();
        return std::get<std::string>(content);
    }

    const std::string& open_sign() const {
        if (!is_signed()) throw HasStuffException();
        return std::get<std::string>(content);
    }

    T& expect(const std::string& failure_msg) {
        if (is_stuffed()) return std::get<T>(content);
        std::cerr << failure_msg << std::endl;
        throw NoStuffException(std::get<std::string>(content));
    }

    std::string& expect_sign(const std::string& failure_msg) {
        if (is_signed()) return std::get<std::string>(content);
        std::cerr << failure_msg << std::endl;
        throw HasStuffException();
    }

    // Mapping
    template<typename F>
    Chest<MapResult<F,T>> map_stuff(F f) const {
        using U = decltype(f(std::declval<T>()));
        if (is_stuffed()) return Chest<U>::stuff(f(std::get<T>(content)));
        return Chest<U>::sign(std::get<std::string>(content));
    }

    template<typename F, typename U = MapResult<F,T>>
    U map_or(const U& backup, F f) const {
        return is_stuffed() ? f(std::get<T>(content)) : backup;
    }

    template<typename F, typename G>
    MapResult<F,T> map_or_fallback(G fallback, F f) const {
        return is_stuffed() ? f(std::get<T>(content)) : fallback(std::get<std::string>(content));
    }

    template<typename F>
    Chest<T> map_sign(F f) const {
        return is_stuffed() ? *this : Chest<T>::sign(f(std::get<std::string>(content)));
    }

    Chest<T> sign_append(const std::string& extra) const {
        return is_stuffed() ? *this : Chest<T>::sign(std::get<std::string>(content) + extra);
    }

    Chest<T> sign_append_newline(const std::string& extra) const {
        return is_stuffed() ? *this : Chest<T>::sign(std::get<std::string>(content) + "\n" + extra);
    }

    Chest<T*> as_ptr() const {
        if (is_stuffed()) return Chest<T*>::stuff(&std::get<T>(content));
        return Chest<T*>::sign(std::get<std::string>(content));
    }

    Chest<std::reference_wrapper<T>> as_ref() const {
        if (is_stuffed()) return Chest<std::reference_wrapper<T>>::stuff(std::ref(std::get<T>(content)));
        return Chest<std::reference_wrapper<T>>::sign(std::get<std::string>(content));
    }
};