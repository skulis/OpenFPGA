cmake_minimum_required(VERSION 2.8.12)

if (${CMAKE_VERSION} VERSION_GREATER "3.8")
    #For cmake >= 3.9 INTERPROCEDURAL_OPTIMIZATION behaviour we need to explicitly
    #set the cmake policy version number
    cmake_policy(VERSION 3.9) 

    # If we are using verison < 3.9 then setting INTERPROCEDURAL_OPTIMIZATION
    # has no effect unless an Intel compiler is used
endif()

set(CMAKE_BUILD_TYPE=Release)

project("libpcre")

#Collect the source files
file(GLOB_RECURSE EXEC_SOURCES SRC/main.c)
file(GLOB_RECURSE LIB_SOURCES SRC/*.c)
file(GLOB_RECURSE LIB_HEADERS SRC/*.h)
files_to_dirs(LIB_HEADERS LIB_INCLUDE_DIRS)

# Use c++ compiler for c source files
set_source_files_properties(${LIB_SOURCES} PROPERTIES LANGUAGE CXX)
set_source_files_properties(${EXEC_SOURCES} PROPERTIES LANGUAGE CXX)
set_source_files_properties(${EXEC_SOURCES_SHELL} PROPERTIES LANGUAGE CXX)

# Remove test executable from library
list(REMOVE_ITEM LIB_SOURCES ${EXEC_SOURCES}) 

#Create the library
add_library(libpcre STATIC
            ${LIB_HEADERS}
            ${LIB_SOURCES})

# add header files to be included
target_include_directories(libpcre PUBLIC ${LIB_INCLUDE_DIRS})
set_target_properties(libpcre PROPERTIES PREFIX "") #Avoid extra 'lib' prefix#Create the executable

#Create the executable
add_executable(pcredemo ${EXEC_SOURCES})
target_link_libraries(pcredemo libpcre)

