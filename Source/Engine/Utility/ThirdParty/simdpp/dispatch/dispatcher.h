/*  Copyright (C) 2013  Povilas Kanapickas <povilas@radix.lt>

    Distributed under the Boost Software License, Version 1.0.
        (See accompanying file LICENSE_1_0.txt or copy at
            http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef LIBSIMDPP_DISPATCHER_H
#define LIBSIMDPP_DISPATCHER_H

#ifndef LIBSIMDPP_SIMD_H
    #error "This file must be included through simd.h"
#endif

#include <algorithm>
#include <cstdlib>
#include <simdpp/dispatch/arch.h>

namespace simdpp {

/** @def SIMDPP_USER_ARCH_INFO
    The user must define this macro if he wants to use the dispatcher
    infrastructure. The macro must evaluate to a constant expression that could
    implicitly initialize an object of type @c std::function<Arch()>.

    The macro is used by @c SIMDPP_MAKE_DISPATCHER_* to specify function which
    is invoked to determine which version of the dispatched function to
    dispatch. The macro is just a more convenient method to pass a parameter
    which has high chance to be the same in the majority of use cases. The user
    may redefine the macro and use different definitions for each site of
    @c SIMDPP_MAKE_DISPATCHER_* expansion.

    The function identified by the @c SIMDPP_USER_ARCH_INFO is called at the
    first time the specific dispatcher is invoked. The user must ensure that
    proper synchronization is used if the dispatcher is called concurrently.

    The user must ensure that the returned information is sensible: e.g. SSE2
    must be supported if SSE3 support is indicated.

    The @c simdpp/dispatch/get_arch_*.h files provide several ready
    implementations of CPU features detection.
*/

namespace detail {

using VoidFunPtr = void (*)();

struct FnVersion {
    /*  Identifies the instruction support that is needed for this version to
        run.
    */
    Arch needed_arch;

    /*  The type-erased function pointer. Only non-member functions are
        supported.

        We type-erase this pointer to reduce the code bloat by allowing @c
        select_version to be shared across all dispatchers.

        Note, that we can't reinterpret_cast to void*, as that is undefined
        behavior in C++. reinterpret_cast to another function pointer
        type is allowed fortunately.
    */
    VoidFunPtr fun_ptr;

    /*  Optional string identifier identifying the architecture. */
    const char* arch_name;
};

inline FnVersion select_version_any(FnVersion* versions, unsigned size,
                                    Arch arch)
{
    // No need to try to be very efficient here.
    std::sort(versions, versions + size,
              [](const FnVersion& lhs, const FnVersion& rhs) {
                  return lhs.needed_arch > rhs.needed_arch;
              });

    unsigned i;
    for (i = 0; i < size; ++i) {
        if (versions[i].fun_ptr == nullptr)
            continue;
        if (test_arch_subset(arch, versions[i].needed_arch)) {
            break;
        }
    }
    if (i == size) {
        // The user didn't provide the NONE_NULL version and no SIMD
        // architecture is supported. We can't do anything except to abort
        std::abort();
    }
    return versions[i];
}

} // namespace detail
} // namespace simdpp

#endif
