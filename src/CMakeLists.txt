
add_library(libChess benchmark.cpp bitops.cpp book.cpp command.cpp data.cpp endgame.cpp eval.cpp game.cpp hashKey.cpp vajo_io.cpp libchess.cpp magicmoves.cpp Move.cpp movegen.cpp movepicker.cpp parameters.cpp perft.cpp polyglotKey.cpp position.cpp search.cpp searchData.cpp see.cpp thread.cpp timeManagement.cpp transposition.cpp uciParameters.cpp syzygy/tbprobe.cpp syzygy/tbfile.cpp syzygy/tbtable.cpp)

add_executable(tuner tuner.cpp )
target_link_libraries (tuner libChess)
add_executable(Vajolet vajolet.cpp )
target_link_libraries (Vajolet libChess)



# Make sure the compiler can find include files for our libChess library
# when other libraries or executables link to libChess
target_include_directories (libChess PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})