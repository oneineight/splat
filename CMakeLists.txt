cmake_minimum_required(VERSION 3.10)

project(splat VERSION 2.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-O3 -Wall -ffast-math")
set(CMAKE_C_FLAGS "-O3 -Wall -ffast-math")	# required for optimization of itwom3.c until it is ported to C++

if(WIN32)
  add_definitions(-DHAVE_LIBPNG)
  set(SHAREDIR ".")
  set(DOCDIR "docs")
elseif(APPLE)
  add_definitions(-DHAVE_LIBPNG -DHAVE_LIBJPEG -DHAVE_LIBGDAL)
  set(SHAREDIR "${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}")
  set(DOCDIR "share/docs/${PROJECT_NAME}")
  set(MANDIR "share/man")
elseif(UNIX) # Linux, BSD etc
  add_definitions(-DHAVE_LIBPNG -DHAVE_LIBJPEG -DHAVE_LIBGDAL)
  set(SHAREDIR "share/${PROJECT_NAME}")
  set(DOCDIR "share/docs/${PROJECT_NAME}")
  set(MANDIR "share/man")
endif()


add_subdirectory(src)
add_subdirectory(utils)
add_subdirectory(docs)

install(DIRECTORY sample_data DESTINATION "${SHAREDIR}")
