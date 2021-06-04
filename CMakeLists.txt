cmake_minimum_required(VERSION 3.16)
project(cpplinq LANGUAGES CXX VERSION 0.1.0.0)

option(CPPLINQ_BUILD_UNIT_TESTS "Builds unit tests" ON)

if (CPPLINQ_BUILD_UNIT_TESTS)
	add_subdirectory(tests)
endif()

set(HDR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(${PROJECT_NAME} INTERFACE)
add_library(cpplinq::cpplinq ALIAS ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} 
	PROPERTIES  
	CXX_STANDARD 11
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF)
target_include_directories(${PROJECT_NAME} INTERFACE ${HDR_DIR})
