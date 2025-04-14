cmake_minimum_required(VERSION 3.10)
project(SciPyFilter VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置Eigen路径（位于3rdparty目录下）
set(EIGEN3_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/Eigen")
message(STATUS "Using Eigen from: ${EIGEN3_INCLUDE_DIR}")
include_directories(${EIGEN3_INCLUDE_DIR})

# Add source files
set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
file(GLOB SOURCE_FILES 
    ${SOURCE_DIR}/butter.cpp
    ${SOURCE_DIR}/iirfilter.cpp
    ${SOURCE_DIR}/conversions.cpp
    ${SOURCE_DIR}/transformations.cpp 
    ${SOURCE_DIR}/prototype.cpp
    ${SOURCE_DIR}/utilities.cpp
)

add_executable(filtfilt_demo ${SOURCE_DIR}/main.cpp ${SOURCE_FILES})

# Include directories
target_include_directories(filtfilt_demo PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SOURCE_DIR}
)

# Set compile options for warnings
if(MSVC)
    target_compile_options(filtfilt_demo PRIVATE /W4)
else()
    target_compile_options(filtfilt_demo PRIVATE -Wall -Wextra -pedantic)
endif()

# Installation rules
install(TARGETS filtfilt_demo DESTINATION bin)
install(FILES 
    ${SOURCE_DIR}/arraytools.h 
    ${SOURCE_DIR}/signaltools.h
    ${SOURCE_DIR}/filter_design.h
    DESTINATION include/scipy-filter-cpp)

# Tests configuration
option(BUILD_TESTS "Build test programs" ON)

if(BUILD_TESTS)
    message(STATUS "Configuring tests")
    set(TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    
    # Add test_butter_compare executable
    add_executable(test_butter_compare ${TEST_DIR}/test_butter_compare.cpp ${SOURCE_FILES})
    
    # Include directories for tests
    target_include_directories(test_butter_compare PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${SOURCE_DIR}
    )
    
    # Set compile options for tests
    if(MSVC)
        target_compile_options(test_butter_compare PRIVATE /W4)
    else()
        target_compile_options(test_butter_compare PRIVATE -Wall -Wextra -pedantic)
    endif()
    
    message(STATUS "Tests configured successfully")
endif()

# FiltFilt Test Target
add_executable(test_filtfilt_compare ${TEST_DIR}/test_filtfilt_compare.cpp ${SOURCE_FILES})
target_include_directories(test_filtfilt_compare PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SOURCE_DIR}
)
if(MSVC)
    target_compile_options(test_filtfilt_compare PRIVATE /W4)
else()
    target_compile_options(test_filtfilt_compare PRIVATE -Wall -Wextra -pedantic)
endif()

# Print message
message(STATUS "Configuration completed. Use build system to compile the project.")

 
