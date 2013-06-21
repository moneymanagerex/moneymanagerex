# define minimum cmake version
cmake_minimum_required(VERSION 2.6.2)
 
# Our project is called 'sqlite3' this is how it will be called in
# visual studio, and in our makefiles. 
project(lua)

# For convenience we define the sources as a variable. You can add 
# header files and cpp/c files and CMake will sort them out
FILE(GLOB SRCS "src/*.c")
 
# Define the output
add_library(lua STATIC  ${SRCS})
SET_TARGET_PROPERTIES(lua PROPERTIES COMPILE_FLAGS -fPIC)
