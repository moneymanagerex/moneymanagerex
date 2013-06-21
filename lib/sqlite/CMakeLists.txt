# define minimum cmake version
cmake_minimum_required(VERSION 2.6.2)
 
# Our project is called 'sqlite3' this is how it will be called in
# visual studio, and in our makefiles. 
project(sqlite3)

add_definitions(-DSQLITE_CORE)
add_definitions(-DSQLITE_HAS_CODEC)
add_definitions(-DSQLITE_ENABLE_COLUMN_METADATA)

# For convenience we define the sources as a variable. You can add 
# header files and cpp/c files and CMake will sort them out
FILE(GLOB SRCS "sqlite3secure.c")
 
# Define the output
add_library(sqlite3 STATIC  ${SRCS})
SET_TARGET_PROPERTIES( sqlite3 PROPERTIES COMPILE_FLAGS -fPIC)

#install(TARGETS sqlite3 DESTINATION ${PLUGINS_DIR})
