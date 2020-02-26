find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)
find_package(Threads REQUIRED)
include_directories(${PNG_INCLUDE_DIR})
add_executable(splat
    itwom3.0.c
    zip.c
    sysutil.cpp
    imagewriter.cpp
    workqueue.cpp
    splat.cpp
    )
target_link_libraries(splat bz2 ${PNG_LIBRARIES} ${JPEG_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
install(TARGETS splat DESTINATION bin)
