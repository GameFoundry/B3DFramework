#   Copyright (C) 2013  Povilas Kanapickas <povilas@radix.lt>
#
#   Distributed under the Boost Software License, Version 1.0.
#       (See accompanying file LICENSE_1_0.txt or copy at
#           http://www.boost.org/LICENSE_1_0.txt)

file(GLOB_RECURSE HEADERS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h *.inl)

foreach(FILE ${HEADERS})
    get_filename_component(FILE_PATH "${FILE}" PATH)
    install(FILES "${FILE}" DESTINATION "${SIMDPP_INCLUDEDIR}/simdpp/${FILE_PATH}")
endforeach()

# Don't enable header tests by default because configuring it takes excessive
# amount of time
set(ENABLE_HEADER_TESTS "0")

if(${ENABLE_HEADER_TESTS} STREQUAL "1")

    simdpp_get_compilable_archs(COMPILABLE_ARCHS)

    set(HEADER_TESTS "")
    add_custom_target(check_headers)

    foreach(ARCH ${COMPILABLE_ARCHS})
        simdpp_get_arch_info(CXX_FLAGS DEFINES_LIST SUFFIX ${ARCH})
        foreach(FILE ${HEADERS})

            if("${FILE}" STREQUAL ".inl")
                continue()
            endif()

            string(REPLACE "/" "_" TEST "${FILE}")
            string(REPLACE "." "_" TEST "${TEST}")
            set(TEST "${TEST}${SUFFIX}")
            set(TEST_OUT "check_headers/test_header_compiles_${TEST}")
            string(REPLACE "-" "_" TEST_TARGET "check_headers_${TEST}")


            file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/check_headers")
            separate_arguments(CXX_FLAGS)
            add_custom_command(
                OUTPUT ${TEST_OUT}
                COMMAND ${CMAKE_CXX_COMPILER}
                        -DLIBSIMDPP_SIMD_H
                        -I "${CMAKE_SOURCE_DIR}"
                        ${CXX_FLAGS} -x c++ -std=c++11 -g2 -Wall
                        ${CMAKE_SOURCE_DIR}/simdpp/${FILE}
                        -c -o ${CMAKE_BINARY_DIR}/${TEST_OUT}
                DEPENDS ${FILE} )
            add_custom_target(${TEST_TARGET} DEPENDS ${TEST_OUT})
            add_dependencies(check_headers "${TEST_TARGET}")
        endforeach()
    endforeach()

endif()

