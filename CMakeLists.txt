cmake_minimum_required(VERSION 3.15)
project(OrderBook VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set output directories
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build: Debug or Release" FORCE)
endif()

# Convert build type to lowercase for directory naming
string(TOLOWER ${CMAKE_BUILD_TYPE} build_type_lower)

# Set output directories using lowercase build type
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${build_type_lower}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${build_type_lower}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${build_type_lower}/lib)

# Enable warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror -Weffc++ -Wconversion -Wsign-conversion)
endif()

# Find Catch2
find_package(Catch2 REQUIRED)

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

# Library source files
set(ORDERBOOK_SOURCES
    src/orderbook/Order.cpp
    src/orderbook/OrderBook.cpp
    src/orderbook/OrderResult.cpp
    src/orderbook/Trade.cpp
)

# Create the orderbook library
add_library(orderbook ${ORDERBOOK_SOURCES})
target_include_directories(orderbook PUBLIC ${PROJECT_SOURCE_DIR}/include)

# Test source files
set(TEST_SOURCES
    tests/test_main.cpp
    tests/unit/order/order_tests.cpp
    tests/unit/orderbook/basic_tests.cpp
    tests/unit/orderbook/depth_tests.cpp
    tests/unit/orderbook/order_manipulation_tests.cpp
    tests/unit/orderbook/id_tests.cpp
    tests/unit/order_filling/fill_tests.cpp
    tests/unit/order_filling/partial_fill_tests.cpp
    tests/unit/order_filling/time_priority_tests.cpp
    tests/unit/order_filling/walk_the_book_tests.cpp
)

# Create the test executable
add_executable(run_tests ${TEST_SOURCES})
target_include_directories(run_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/tests  # Add tests directory to include path
)
target_link_libraries(run_tests PRIVATE orderbook Catch2::Catch2)

# Register tests with CTest
include(CTest)
include(Catch)
catch_discover_tests(run_tests)

# Install rules
install(TARGETS orderbook
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/orderbook
    DESTINATION include
)
