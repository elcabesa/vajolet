target_sources(Vajolet_tuner PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}/PGNCommentText.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/PGNException.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/PGNGame.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/PGNGameCollection.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/PGNGameResult.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/PGNMove.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/PGNMoveList.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/PGNParser.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/PGNPiece.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/PGNPly.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/PGNPosition.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/PGNSquare.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/PGNTag.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/PGNTagList.cpp)

# Make sure the compiler can find include files for our Vajolet_tuner
# when other libraries or executables link to Vajolet_tuner
target_include_directories (Vajolet_tuner PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
