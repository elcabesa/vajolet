target_sources(libChess PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/differentialList.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/featureList.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/input.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sparse.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/parallelSparse.cpp
)

# Make sure the compiler can find include files for our libChess library
# when other libraries or executables link to libChess
target_include_directories (libChess PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

