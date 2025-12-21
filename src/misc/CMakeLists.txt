target_sources(libChess PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}/bitops.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/data.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/vajo_io.cpp)

# Make sure the compiler can find include files for our libChess library
# when other libraries or executables link to libChess
target_include_directories (libChess PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

