// Copyright (C) 2016 Jonathan Müller <jonathanmueller.dev@gmail.com>
// This file is subject to the license terms in the LICENSE file
// found in the top-level directory of this distribution.

#ifndef TYPE_SAFE_DEFERRED_CONSTRUCTION_HPP_INCLUDED
#define TYPE_SAFE_DEFERRED_CONSTRUCTION_HPP_INCLUDED

#include <new>
#include <type_traits>

#include <type_safe/detail/assert.hpp>

namespace type_safe
{
    /// A tiny wrapper to create an object without constructing it yet.
    ///
    /// This is useful if you have a type that is default constructible,
    /// but can't be initialized properly - yet.
    /// It works especially well with [type_safe::output_parameter<T>]().
    ///
    /// It has two states:
    /// Either it is *intialized* in which case you can get its value,
    /// or it is *un-initialized* in which case you cannot get its value.
    /// All objects start out un-initialized.
    /// For consistency with [type_safe::basic_optional<T>]() it provides a similar interface,
    /// yet it is not as flexible and does not allow to reset it to the uninitialized state,
    /// once initialized.
    ///
    /// \notes Because of the interface similarities,
    /// you can use it with the free functions declared in `optional.hpp`,
    /// such as `with()`, `visit()` and `apply()`.
    template <typename T>
    class deferred_construction
    {
    public:
        using value_type = T;

        //=== constructors/assignment/destructor ===//
        /// \effects Creates it in the un-initialized state.
        deferred_construction() noexcept : initialized_(false)
        {
        }

        /// \effects Copy constructor:
        /// If `other` is un-initialized, it will be un-initialized as well.
        /// If `other` is initialized, it will copy the stored value.
        /// \throws Anything thrown by the copy constructor of `value_type` if `other` is initialized.
        deferred_construction(const deferred_construction& other) : initialized_(other.initialized_)
        {
            if (initialized_)
                ::new (as_void()) value_type(other.value());
        }

        /// \effects Move constructor:
        /// If `other` is un-initialized, it will be un-initialized as well.
        /// If `other` is initialized, it will copy the stored value.
        /// \throws Anything thrown by the move constructor of `value_type` if `other` is initialized.
        /// \notes `other` will still be initialized after the move operation,
        /// it is just in a moved-from state.
        deferred_construction(deferred_construction&& other) noexcept(
            std::is_nothrow_move_constructible<value_type>::value)
        : initialized_(other.initialized_)
        {
            if (initialized_)
                ::new (as_void()) value_type(std::move(other).value());
        }

        /// \notes You cannot construct it from the type directly.
        /// If you are able to do that, there is no need to use `defer_construction~!
        deferred_construction(value_type) = delete;

        /// \effects If it is initialized, it will destroy the value.
        /// Otherwise it has no effect.
        ~deferred_construction() noexcept
        {
            if (initialized_)
                value().~value_type();
        }

        /// \notes You cannot copy or move assign it.
        /// This is a deliberate design decision to guarantee,
        /// that an initialized object stays initialized, no matter what.
        deferred_construction& operator=(deferred_construction) = delete;

        /// \effects Same as `emplace(std::forward<U>(u))`.
        /// \requires `value_type` must be constructible from `U`.
        /// \notes You must not use this function to actually "assign" the value,
        /// like `emplace()`, the object must not be initialized.
        template <typename U>
        auto operator=(U&& u) ->
            typename std::enable_if<std::is_constructible<T, decltype(std::forward<U>(u))>::value,
                                    deferred_construction&>::type
        {
            emplace(std::forward<U>(u));
            return *this;
        }

        //=== modifiers ===//
        /// \effects Initializes the object with the `value_type` constructed from `args`.
        /// \requires `has_value() == false`.
        /// \throws Anything thrown by the chosen constructor of `value_type`.
        /// \notes You must only call this function once,
        /// after the object has been initialized,
        /// you can use `value()` to assign to it.
        template <typename... Args>
        void emplace(Args&&... args)
        {
            DEBUG_ASSERT(!has_value(), detail::assert_handler{});
            ::new (as_void()) value_type(std::forward<Args>(args)...);
            initialized_ = true;
        }

        //=== observers ===//
        /// \returns The same as `has_value()`.
        explicit operator bool() const noexcept
        {
            return has_value();
        }

        /// \returns `true` if the object is initialized, `false` otherwise.
        bool has_value() const noexcept
        {
            return initialized_;
        }

        /// \returns A reference to the stored value.
        /// \requires `has_value() == true`.
        value_type& value() & noexcept
        {
            DEBUG_ASSERT(has_value(), detail::assert_handler{});
            return *static_cast<value_type*>(as_void());
        }

        /// \returns A `const` reference to the stored value.
        /// \requires `has_value() == true`.
        const value_type& value() const& noexcept
        {
            DEBUG_ASSERT(has_value(), detail::assert_handler{});
            return *static_cast<const value_type*>(as_void());
        }

        /// \returns An rvalue reference to the stored value.
        /// \requires `has_value() == true`.
        value_type&& value() && noexcept
        {
            DEBUG_ASSERT(has_value(), detail::assert_handler{});
            return std::move(*static_cast<value_type*>(as_void()));
        }

        /// \returns An rvalue reference to the stored value.
        /// \requires `has_value() == true`.
        const value_type&& value() const&& noexcept
        {
            DEBUG_ASSERT(has_value(), detail::assert_handler{});
            return std::move(*static_cast<const value_type*>(as_void()));
        }

    private:
        void* as_void() noexcept
        {
            return static_cast<void*>(&storage_);
        }

        const void* as_void() const noexcept
        {
            return static_cast<const void*>(&storage_);
        }

        using storage_t = typename std::aligned_storage<sizeof(T), alignof(T)>::type;
        storage_t storage_;
        bool      initialized_;
    };

} // namespace type_safe

#endif // TYPE_SAFE_DEFER_CONSTRUCTION_HPP_INCLUDED
