target_sources(libChess PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/parallelDenseLayer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/denseLayer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/layer.cpp
    )

# Make sure the compiler can find include files for our libChess library
# when other libraries or executables link to libChess
target_include_directories (libChess PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

