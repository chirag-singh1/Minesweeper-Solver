# CMakeList.txt : CMake project for minesweeper, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("minesweeper")

# Add source to this project's executable.
add_executable (minesweeper "main.cpp"  "board.h" "board.cpp" "util.cpp" "bot.h" "bot.cpp" "util.h")

# TODO: Add tests and install targets if needed.
