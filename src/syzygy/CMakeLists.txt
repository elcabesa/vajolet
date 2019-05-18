target_sources(libChess PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}/tbCommonData.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/tbfile.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/tbpairs.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/tbtable.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/tbtables.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/tbtableDTZ.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/tbtableWDL.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/tbvalidater.cpp)

# Make sure the compiler can find include files for our libChess library
# when other libraries or executables link to libChess
target_include_directories (libChess PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

