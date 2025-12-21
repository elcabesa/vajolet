target_sources(libChess PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}/game.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/rootMovesToBeSearched.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/search.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/searchData.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/searcher.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/searchLogger.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/see.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/thread.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/timeManagement.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/transposition.cpp)

# Make sure the compiler can find include files for our libChess library
# when other libraries or executables link to libChess
target_include_directories (libChess PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

