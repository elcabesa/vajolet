# Create a library called "Hello" which includes the source file "hello.cxx".
# The extension is already found. Any number of sources could be listed here.
add_library(libSudoku board.cpp candidates.cpp iterators.cpp solver.cpp tSquares.cpp value.cpp)
add_executable(SudokuSolver main.cpp )
target_link_libraries (SudokuSolver libSudoku)

# Make sure the compiler can find include files for our Hello library
# when other libraries or executables link to Hello
target_include_directories (libSudoku PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})