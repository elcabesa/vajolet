#cmake configuration

cmake_minimum_required (VERSION 3.12)

#project configuration
project (Vajolet)

IF( VAJOLET_CPU_TYPE STREQUAL "64OLD")
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse3 -m64" )
ELSEIF( VAJOLET_CPU_TYPE STREQUAL "64NEW")
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2 -m64 -mpopcnt" )
ELSEIF( VAJOLET_CPU_TYPE STREQUAL "64BMI2")
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.2 -m64 -mbmi -mbmi2 -mpopcnt" )
ELSE()
ENDIF()

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -pedantic -Wall -Wextra" )
if (WIN32)
	if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -femulated-tls" )
	endif()
endif()

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG ")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 ")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --coverage")

if (WIN32)
	if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		set (CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -femulated-tls")
		set (CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -femulated-tls")
	endif()

	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -s -Wl,--allow-multiple-definition -static -pthread")
	set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -Wl,--allow-multiple-definition -static -pthread")		
else()
	
	if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
		set (CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -s -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -static")
		set (CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -static --coverage")
	elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		set (CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -s -lpthread")
		set (CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -s -lpthread")
	else()
		set (CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -s -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -static")
		set (CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -static --coverage")
	endif()
	
endif()

SET(BUILD_GMOCK OFF CACHE BOOL "don't build GMOCK")

set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1)

# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
  RESULT_VARIABLE result
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
				 ${CMAKE_BINARY_DIR}/googletest-build)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
  include_directories("${gtest_SOURCE_DIR}/include")
endif()

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(tuner)
