if (fftw::fftw3)
  return()
endif()

find_path(FFTW_INCLUDE
  NAMES fftw3.h
)

find_library(FFTW_SHARED_LIBRARY
  NAMES libfftw3.so
)

find_library(FFTW_STATIC_LIBRARY
  NAMES libfftw3.a
)

find_package(PackageHandleStandardArgs REQUIRED)

find_package_handle_standard_args(fftw::fftw3
  REQUIRED_VARS FFTW_INCLUDE
                FFTW_SHARED_LIBRARY
)

add_library(fftw::fftw3 SHARED IMPORTED GLOBAL)

set_target_properties(fftw::fftw3 PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${FFTW_INCLUDE}
  IMPORTED_IMPLIB ${FFTW_SHARED_LIBRARY}
  IMPORTED_LOCATION ${FFTW_SHARED_LIBRARY} 
)

# todo: do the long one as well
add_library(fftw::fftw3f SHARED IMPORTED GLOBAL)

set_target_properties(fftw::fftw3f PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${FFTW_INCLUDE}
  IMPORTED_IMPLIB ${FFTW_SHARED_LIBRARY}
  IMPORTED_LOCATION ${FFTW_SHARED_LIBRARY} 
)
target_link_libraries(fftw::fftw3f
  INTERFACE fftw3f
)

