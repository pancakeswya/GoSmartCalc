cmake_minimum_required(VERSION 3.26)

set(CMAKE_CXX_STANDARD 20)
if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra -O3")
endif(UNIX)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories("third_party/DateTime/src")
include_directories(".")

add_library(DateTime STATIC
                third_party/DateTime/src/timespan.cpp
                third_party/DateTime/src/timespan.h
                third_party/DateTime/src/datetime.cpp
                third_party/DateTime/src/datetime.h)

add_library(CalcCore SHARED
            core/basic_calc.cc
            core/basic_calc.h
            core/credit_calc.cc
            core/credit_calc.h
            core/deposit_calc.cc
            core/deposit_calc.h)

target_link_libraries(CalcCore DateTime)

set_target_properties(CalcCore PROPERTIES OUTPUT_NAME "calc_core")

add_compile_definitions(CALC_EXPORT)