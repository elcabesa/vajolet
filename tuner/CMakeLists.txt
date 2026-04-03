add_executable(Vajolet_genFen
	binSaver.cpp
	book.cpp
	clock.cpp
	elo.cpp
	genFen.cpp
	player.cpp
	selfplay.cpp
	tournament.cpp
	tunerPars.cpp
	txtSaver.cpp)

target_link_libraries(Vajolet_genFen  libChess)

add_executable(Vajolet_verifyTrainDataFeature
	verifyTrainDataFeature.cpp
	)

target_link_libraries(Vajolet_verifyTrainDataFeature  libChess)

add_executable(Vajolet_stats
	stats.cpp
	)

target_link_libraries(Vajolet_stats  libChess)


target_include_directories (Vajolet_unit_test PRIVATE ${CMAKE_SOURCE_DIR}/src)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/book.pgn
    ${CMAKE_CURRENT_BINARY_DIR}/book.pgn COPYONLY)

add_executable(Vajolet_relabel
	relabel.cpp
	player.cpp
	elo.cpp
	tunerPars.cpp
	)

target_link_libraries(Vajolet_relabel  libChess)

add_subdirectory(pgnlib-0.2.5)
