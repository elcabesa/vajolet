target_sources(libChess PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}/magicmoves.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/move.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/movegen.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/movepicker.cpp)

# Make sure the compiler can find include files for our libChess library
# when other libraries or executables link to libChess
target_include_directories (libChess PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

