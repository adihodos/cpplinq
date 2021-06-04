cmake_minimum_required(VERSION 3.16)
project(cpplinq-tests LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(
	catch2
	GIT_REPOSITORY https://github.com/catchorg/Catch2.git
	GIT_TAG v2.13.6
)

FetchContent_MakeAvailable(catch2)

set(SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/main.cc)
add_executable(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE cpplinq::cpplinq Catch2::Catch2)
set_target_properties(
	${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 11
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF)