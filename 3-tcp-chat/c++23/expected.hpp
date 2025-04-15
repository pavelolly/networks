#pragma once

#if _HAS_CXX23

#include <expected>

template<class T, class E>
using Expected = std::expected<T, E>;

template<class E>
using Unexpected = std::unexpected<E>;

template<class E>
using BadExpectedAccess = std::bad_expected_access<E>;

#else

#include <exception>

template<class E>
class BadExpectedAcess : public BadExpectedAcess<void> {
public:
    BadExpectedAcess(E error) : error_(std::move(error)) {}

    E &error() {
        return error_;
    }

    const E &error() const {
        return error_;
    }
private:
    E error_;
};

template<>
class BadExpectedAcess<void> : public std::exception {};


template<class E>
class Unexpected {
public:
    constexpr Unexpected(E value) : value_(std::move(value))
    {}

    constexpr Unexpected(const Unexpected &other) : value_(other.value_) {}
    constexpr Unexpected(Unexpected &&other) : value_(std::move(other.value_)) {}

    constexpr E &error() {
        return value_;
    }

    constexpr const E &error() const {
        return value_;
    }
private:
    E value_;
};

template<class T, class E>
class Expected {
public:
    constexpr Expected(T expected) : expected_(std::move(expected)), bExpected(true)
    {}
    constexpr Expected(Unexpected<E> unexpected) : unexpected_(unexpected.error()), bExpected(false)
    {}

    constexpr T &value() {
        if (!bExpected) {
            throw BadExpectedAcess(unexpected_);
        }
        return expected_;
    }

    constexpr const T &value() const {
        return const_cast<Expected *>(this)->value();
    }

    constexpr E &error() {
        return unexpected_;
    }

    constexpr const E &error() const {
        return unexpected_;
    }

    constexpr bool has_value() const {
        return bExpected;
    }

    constexpr explicit operator bool() const {
        return bExpected;
    }
private:
    T expected_;
    E unexpected_;
    bool bExpected;
};

#endif