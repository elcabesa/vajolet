
# Now simply link against gtest or gtest_main as needed. Eg
add_executable(Vajolet_tuner 
	clock.cpp
	selfplay.cpp
	tournament.cpp
	tuner.cpp)

target_link_libraries(Vajolet_tuner  libChess)
target_include_directories (Vajolet_unit_test PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_subdirectory(pgnlib-0.2.5)
