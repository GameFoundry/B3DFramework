/*  Copyright (C) 2013-2014  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_CORE_ALIGNED_ALLOCATOR_H
#define LIBSIMDPP_CORE_ALIGNED_ALLOCATOR_H

#include <memory>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace simdpp {
namespace SIMDPP_ARCH_NAMESPACE {

/** An allocator that allocates memory with stricter alignment requirements than
    the defaults. @a A must be a power of two.
*/
template<class T, std::size_t A>
class aligned_allocator {
private:

    static_assert(!(A & (A - 1)), "A is not a power of two");

public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    aligned_allocator() = default;
    aligned_allocator(const aligned_allocator&) = default;

    template<class U>
    aligned_allocator(const aligned_allocator<U,A>&) {}

    ~aligned_allocator() = default;

    aligned_allocator& operator=(const aligned_allocator&) = delete;

    template<class U>
    struct rebind {
        using other = aligned_allocator<U,A>;
    };

    T* address(T& x) const
    {
        return &x;
    }

    std::size_t max_size() const
    {
        return (static_cast<std::size_t>(0) - static_cast<std::size_t>(1)) / sizeof(T);
    }

    // stateless
    bool operator!=(const aligned_allocator&) const { return false; }
    bool operator==(const aligned_allocator&) const { return true; }

    void construct(T* p, const T& t) const
    {
        void* pv = static_cast<void*>(p);
        new (pv) T(t);
    }

    void destroy(T* p) const
    {
        p->~T();
    }

    T* allocate(std::size_t n) const
    {
        if (n == 0) {
            return nullptr;
        }

		if (n > max_size()) {
            //BSF: replaced throw with a simple abort, since in `-fno-exceptions` mode the compiler would put
            // an abort in here anyway.
            //throw std::length_error("aligned_allocator<T,A>::allocate() - Integer overflow.");
            abort();
		}


        /* We align the storage by adding @a alignment bytes and truncating the
            pointer. The pointer to the original location returned by @a new is
            stored just before the location the returned pointer refers to.
            To ensure that there is always at least @a sizeof(void*) space
            there, @a alignment is at least 2*sizoef(void*)
        */
        std::size_t al = A < 2*sizeof(void*) ? 2*sizeof(void*) : A;

        char* pv = new char[n*sizeof(T) + al];
        std::uintptr_t upv = reinterpret_cast<std::uintptr_t>(pv);
        upv = (upv + al) & ~(al - 1);
        char** aligned_pv = reinterpret_cast<char**>(upv);

        *(aligned_pv-1) = pv; // original pointer

        return reinterpret_cast<T*>(aligned_pv);
    }

    void deallocate(T* p, std::size_t n) const
    {
        (void) n;
        if (!p) {
            return;
        }
        char** pptr = reinterpret_cast<char**>(p);
        delete[](*(pptr - 1));
    }

    template<class U>
    T * allocate(std::size_t n, const U* hint) const
    {
        (void) hint;
        return allocate(n);
    }
};

} // namespace SIMDPP_ARCH_NAMESPACE
} // namespace simdpp

#endif
