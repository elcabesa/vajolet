target_sources(libChess PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/diskInputSet.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/inputSet.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/labeledExample.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/memoryInputSet.cpp
    
)

# Make sure the compiler can find include files for our libChess library
# when other libraries or executables link to libChess
target_include_directories (libChess PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

