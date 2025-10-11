
# Now simply link against gtest or gtest_main as needed. Eg
add_executable(Vajolet_tuner 
	binSaver.cpp
	book.cpp
	clock.cpp
	elo.cpp
	epdSaver.cpp
	fenSaver.cpp
	player.cpp
	selfplay.cpp
	spsa.cpp
	tournament.cpp
	tuner.cpp
	tunerPars.cpp)

target_link_libraries(Vajolet_tuner  libChess)

add_executable(Vajolet_genFen
	binSaver.cpp
	book.cpp
	clock.cpp
	elo.cpp
	epdSaver.cpp
	fenSaver.cpp
	genFen.cpp
	player.cpp
	selfplay.cpp
	tournament.cpp
	tunerPars.cpp)

target_link_libraries(Vajolet_genFen  libChess)

add_executable(Vajolet_genTrainData
	genTrainData.cpp
	)

target_link_libraries(Vajolet_genTrainData  libChess)

add_executable(Vajolet_genTrainDataFeature
	genTrainDataFeature.cpp
	)

target_link_libraries(Vajolet_genTrainDataFeature  libChess)

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

add_subdirectory(pgnlib-0.2.5)
