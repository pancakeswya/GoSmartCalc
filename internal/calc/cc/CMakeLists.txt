cmake_minimum_required(VERSION 3.26)

set(CMAKE_C_STANDARD 11)
if(UNIX)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -Wextra -O3")
endif(UNIX)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(CalcCore SHARED
            api.h
            basic_calc.c
            basic_calc.h
            credit_calc.c
            credit_calc.h
            defs.h
            deposit_calc.c
            deposit_calc.h
            util/date.h
            util/math_operation.h
            util/stack_double.c
            util/stack_double.h
            util/stack_operation.c
            util/stack_operation.h
            util/str_util.c
            util/str_util.h
            util/vector.h
)

find_library(MATH_LIBRARY m)
if(MATH_LIBRARY)
    target_link_libraries(CalcCore PUBLIC ${MATH_LIBRARY})
endif()

set_target_properties(CalcCore PROPERTIES OUTPUT_NAME "calc")

add_compile_definitions(CALC_SHARED CALC_EXPORT)