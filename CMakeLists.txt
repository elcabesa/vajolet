cmake_minimum_required (VERSION 2.8)
project (Vajolet)

set(CMAKE_CXX_FLAGS "-std=c++11 -O3 -msse4.2 -mpopcnt -pedantic -Wall -Wextra -fmessage-length=0")
set (CMAKE_EXE_LINKER_FLAGS "-s -pthread")

add_library(libChess benchmark.cpp bitops.cpp book.cpp command.cpp data.cpp eval.cpp hashKeys.cpp io.cpp magicmoves.cpp movegen.cpp position.cpp search.cpp see.cpp thread.cpp transposition.cpp syzygy/tbprobe.cpp)

add_executable(Vajolet vajolet.cpp )
target_link_libraries (Vajolet libChess)

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

    enable_testing()
    # Now simply link against gtest or gtest_main as needed. Eg
    add_executable(Vajolet_test tests/main.cpp tests/test.cpp)
    target_link_libraries(Vajolet_test gtest libChess)
    add_test(NAME example_test COMMAND Vajolet_test)
    
 
    add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS Vajolet_test)