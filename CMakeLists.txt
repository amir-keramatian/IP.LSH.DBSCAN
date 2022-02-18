cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(LSHDBSCAN)

if(NOT CMAKE_BUILD_TYPE)
 set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS "-std=c++11 -O3")
message(STATUS "**************Build type: ${CMAKE_BUILD_TYPE}************")


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")

include_directories(src/include)

file (GLOB LIB_SOURCES src/*.cc)
file (GLOB LIB_HEADERS src/include/*.h)

add_library(myLib ${LIB_SOURCES} ${LIB_HEADERS})

add_executable (Vanila_DBSCAN_LSH_exec apps/main_vanilla_dbscan_lsh.cc)
target_link_libraries(Vanila_DBSCAN_LSH_exec myLib gsl gslcblas gomp tbb pthread)

add_executable (Vanila_DBSCAN_exec apps/main_vanilla_dbscan.cc)
target_link_libraries(Vanila_DBSCAN_exec myLib gsl gslcblas gomp tbb pthread)

add_executable (LSHDBSCAN_exec apps/main_general.cc)
target_link_libraries(LSHDBSCAN_exec myLib gsl gslcblas gomp tbb pthread)

add_executable (ParameterExploration_exec apps/main_parameter_exploration.cc)
target_link_libraries(ParameterExploration_exec myLib gsl gslcblas gomp tbb pthread)
