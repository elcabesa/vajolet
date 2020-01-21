add_library(libChess 
	benchmark.cpp
	bitops.cpp 
	book.cpp
	command.cpp
	data.cpp
	endgame.cpp
	eval.cpp
	game.cpp
	hashKey.cpp
	libchess.cpp
	magicmoves.cpp
	move.cpp
	movegen.cpp
	movepicker.cpp
	parameters.cpp
	perft.cpp
	polyglotKey.cpp
	position.cpp
	rootMovesToBeSearched.cpp
	search.cpp
	searchData.cpp
	searchLogger.cpp
	see.cpp
	thread.cpp
	timeManagement.cpp
	transposition.cpp
	uciParameters.cpp
	uciOutput.cpp
	vajo_io.cpp)
add_subdirectory(syzygy)


add_executable(tuner tuner.cpp )
target_link_libraries (tuner libChess)
add_executable(Vajolet vajolet.cpp )
target_link_libraries (Vajolet libChess)



# Make sure the compiler can find include files for our libChess library
# when other libraries or executables link to libChess
target_include_directories (libChess PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})