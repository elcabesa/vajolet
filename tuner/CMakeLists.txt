
# Now simply link against gtest or gtest_main as needed. Eg
add_executable(Vajolet_tuner 
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

target_link_libraries(Vajolet_tuner  libChess floatfann_static ${OpenMP_CXX_FLAGS})

add_executable(Vajolet_genFen 
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

target_link_libraries(Vajolet_genFen  libChess floatfann_static ${OpenMP_CXX_FLAGS})

add_executable(Vajolet_genTrainData
	book.cpp
	clock.cpp
	elo.cpp
	epdSaver.cpp
	fenSaver.cpp
	genTrainData.cpp
	player.cpp
	selfplay.cpp
	tournament.cpp
	tunerPars.cpp
	)

target_link_libraries(Vajolet_genTrainData  libChess floatfann_static ${OpenMP_CXX_FLAGS})

add_executable(Vajolet_genTrainDataFeature
	genTrainDataFeature.cpp
	)

target_link_libraries(Vajolet_genTrainDataFeature  libChess floatfann_static ${OpenMP_CXX_FLAGS})

add_executable(Vajolet_train
	FANNtrain.cpp)

target_link_libraries(Vajolet_train  floatfann_static ${OpenMP_CXX_FLAGS})

add_executable(Vajolet_train2
	FANNtrain2.cpp)

target_link_libraries(Vajolet_train2 libChess floatfann_static ${OpenMP_CXX_FLAGS})

target_include_directories (Vajolet_unit_test PRIVATE ${CMAKE_SOURCE_DIR}/src)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/book.pgn
    ${CMAKE_CURRENT_BINARY_DIR}/book.pgn COPYONLY)

add_subdirectory(pgnlib-0.2.5)
