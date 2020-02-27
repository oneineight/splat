find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)
find_package(Threads REQUIRED)
include_directories(${PNG_INCLUDE_DIR})
add_executable(splat
    anf.cpp
    boundary_file.cpp
    city_file.cpp
    dem.cpp
    elevation_map.cpp
    gnuplot.cpp
    itwom3.0.cpp
    kml.cpp
    lrp.cpp
    main.cpp
    pat_file.cpp
    path.cpp
    ppm.cpp
    region.cpp
    report.cpp
    sdf.cpp
    site.cpp
    splat_run.cpp
    udt.cpp
    utilities.cpp
    workqueue.cpp)
target_link_libraries(splat bz2 ${PNG_LIBRARIES} ${JPEG_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
install(TARGETS splat DESTINATION bin)
