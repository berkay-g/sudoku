cmake_minimum_required(VERSION 3.14)
project(sudoku)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCE_FILES 
	${CMAKE_SOURCE_DIR}/src/*.c
	${CMAKE_SOURCE_DIR}/src/*.cpp)
	
file(GLOB_RECURSE HEADER_FILES 
	${CMAKE_SOURCE_DIR}/src/*.h)

file(GLOB_RECURSE THIRD_PARTY 
	${CMAKE_SOURCE_DIR}/thirdparty/*.c
	${CMAKE_SOURCE_DIR}/thirdparty/*.cpp)

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES} ${THIRD_PARTY})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include/)

# win32 static
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/lib/SDL3-static.lib winmm.lib imm32.lib version.lib setupapi.lib)

# win32 shared
# target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/lib/SDL3.lib)

if (MSVC)
	message("MSVC")
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	target_link_options(${PROJECT_NAME} PUBLIC "/SUBSYSTEM:WINDOWS")
	target_link_options(${PROJECT_NAME} PUBLIC "/ENTRY:mainCRTStartup")
endif()
