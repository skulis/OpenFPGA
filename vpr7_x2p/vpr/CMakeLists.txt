cmake_minimum_required(VERSION 2.8.12)

if (${CMAKE_VERSION} VERSION_GREATER "3.8")
    #For cmake >= 3.9 INTERPROCEDURAL_OPTIMIZATION behaviour we need to explicitly
    #set the cmake policy version number
    cmake_policy(VERSION 3.9)

    # If we are using verison < 3.9 then setting INTERPROCEDURAL_OPTIMIZATION
    # has no effect unless an Intel compiler is used
endif()

project("vpr7_x2p" C CXX)

# idenify if we need graphics
set(ENABLE_VPR_GRAPHIC_CXX_FLAG true)
message(STATUS "Checking VPR graphics option ${ENABLE_VPR_GRAPHICS}")
if (ENABLE_VPR_GRAPHICS)
  # check for dependencies
  message(STATUS "VPR graphics is turned on, searching for dependencies")
  find_package(X11)

  if (NOT X11_FOUND)
    message(WARNING "Failed to find required X11 library (on debian/ubuntu try 'sudo apt-get install libx11-dev' to install)")
    #Disable
    set(ENABLE_VPR_GRAPHIC_CXX_FLAG false)
  endif()
else ()
  set(ENABLE_VPR_GRAPHIC_CXX_FLAG false)
endif()

if (NOT ENABLE_VPR_GRAPHIC_CXX_FLAG)
  # Add a flag to notify compiler not to consider graphic-related source codes
  set (DISABLE_GRAPHIC_FLAGS "-DNO_GRAPHICS")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${DISABLE_GRAPHIC_FLAGS}")
  message(STATUS "Add flags to disable graphics in VPR compilation: ${DISABLE_GRAPHIC_FLAGS}")
endif()

# We need readline to compile
find_package(Readline REQUIRED)

#Collect the source files
file(GLOB_RECURSE EXEC_SOURCES SRC/main.c)
file(GLOB_RECURSE EXEC_SOURCES_SHELL SRC/shell_main.c)
file(GLOB_RECURSE LIB_SOURCES SRC/*/*.c SRC/*/*/*.c SRC/*/*.cpp SRC/*/*/*.cpp)
file(GLOB_RECURSE LIB_HEADERS SRC/*/*.h SRC/*/*/*.h)
files_to_dirs(LIB_HEADERS LIB_INCLUDE_DIRS)

# Use c++ compiler for c source files
set_source_files_properties(${LIB_SOURCES} PROPERTIES LANGUAGE CXX)
set_source_files_properties(${EXEC_SOURCES} PROPERTIES LANGUAGE CXX)
set_source_files_properties(${EXEC_SOURCES_SHELL} PROPERTIES LANGUAGE CXX)

#Create the library
add_library(libvpr STATIC
            ${LIB_HEADERS}
            ${LIB_SOURCES})

# add header files to be included
target_include_directories(libvpr PUBLIC ${LIB_INCLUDE_DIRS})
set_target_properties(libvpr PROPERTIES PREFIX "") #Avoid extra 'lib' prefix#Create the executable

#Specify link-time dependancies
if (ENABLE_VPR_GRAPHIC_CXX_FLAG)
  target_link_libraries(libvpr
                        libarchfpgavpr7
                        X11
                        libvtrutil
                        libini
                        readline) 
else ()
  target_link_libraries(libvpr
                        libarchfpgavpr7
                        libvtrutil
                        libini
                        readline)
endif()

#Create the executables
# regular vpr interface
add_executable(vpr ${EXEC_SOURCES})
target_link_libraries(vpr
                      libvpr)

# Shell-interface vpr
add_executable(vpr_shell ${EXEC_SOURCES_SHELL})
target_link_libraries(vpr_shell
                      libvpr)

