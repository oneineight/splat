find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)
find_package(Threads REQUIRED)
include_directories(${PNG_INCLUDE_DIR})
add_executable(splat
    itwom3.0.c
    splat.cpp
    sysutil.cpp
    zip.c)
target_link_libraries(splat bz2 ${PNG_LIBRARIES} ${JPEG_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
install(TARGETS splat DESTINATION bin)
