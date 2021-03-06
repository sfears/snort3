cmake_minimum_required ( VERSION 2.8.11 )
project ( alert_unixsock CXX )

if ( APPLE )
    set ( CMAKE_MACOSX_RPATH OFF )
    set (
        CMAKE_SHARED_LINKER_FLAGS
        "${CMAKE_SHARED_LINKER_FLAGS} -undefined dynamic_lookup"
    )
endif ( APPLE )

include ( FindPkgConfig )
pkg_search_module ( SNORT3 REQUIRED snort>=3 )

add_library (
    alert_unixsock SHARED
    alert_unixsock.cc
)

execute_process(
    COMMAND pkg-config --variable=daq_cppflags snort
    OUTPUT_VARIABLE DAQ_CPPFLAGS
)

string(REGEX REPLACE "[\r\n]" " " DAQ_CPPFLAGS "${DAQ_CPPFLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 ${DAQ_CPPFLAGS}")

target_include_directories (
    alert_unixsock PUBLIC
    ${SNORT3_INCLUDE_DIRS}
)

install (
    TARGETS alert_unixsock
    LIBRARY DESTINATION "lib/${CMAKE_PROJECT_NAME}/loggers"
)
