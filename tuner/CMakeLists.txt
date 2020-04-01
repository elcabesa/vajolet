
# Now simply link against gtest or gtest_main as needed. Eg
add_executable(Vajolet_tuner 
	book.cpp
	clock.cpp
	elo.cpp
	player.cpp
	selfplay.cpp
	spsa.cpp
	tournament.cpp
	tuner.cpp)

target_link_libraries(Vajolet_tuner  libChess)
target_include_directories (Vajolet_unit_test PRIVATE ${CMAKE_SOURCE_DIR}/src)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/book.pgn
    ${CMAKE_CURRENT_BINARY_DIR}/book.pgn COPYONLY)

add_subdirectory(pgnlib-0.2.5)
