# ----------------------------------------------------------------------------
# Root CMake file for libtssipython
#
#   Invoke
#      $ cmake <PATH_TO_LIBTS13818PYTHON_ROOT>
#   from off-tree build directory
#
# ----------------------------------------------------------------------------

cmake_minimum_required (VERSION 2.8.8)
project (libtssipython)
set(CMAKE_BUILD_TYPE Release)

find_package(BOOST REQUIRED python)
find_package(PythonLibs 2.7 REQUIRED)

# searching for include directory
find_path(LIBTSSI_INCLUDE_DIR tssi.h)

# searching for library file
find_library(LIBTSSI_LIBRARY libtssi)

link_directories (${Boost_LIBRARY_DIRS})
add_library(libtssipython SHARED tssipython.cpp)

if (LIBTSSI_INCLUDE_DIR AND LIBTSSI_LIBRARY)
    set(LIBTSSI_FOUND TRUE)

    include_directories(
	${LIBTSSI_INCLUDE_DIR}
	${Boost_INCLUDE_DIRS}
        ${PYTHON_INCLUDE_DIRS}
    )

    set(LIBS ${LIBTSSI_LIBRARY} ${Boost_LIBRARIES} ${PYTHON_LIBRARIES})

    set_property(TARGET libtssipython PROPERTY POSITION_INDEPENDENT_CODE 1)
    target_link_libraries( libtssipython ${LIBS})

else()
    MESSAGE( FATAL_ERROR "libtssi not found." )
endif ()














