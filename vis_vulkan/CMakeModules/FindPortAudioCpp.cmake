if (portaudio::portaudiocpp)
  return()
endif()

find_package(PackageHandleStandardArgs REQUIRED)

find_package(PkgConfig)

find_package(portaudio)

if (PKG_CONFIG_FOUND)
  pkg_check_modules(PACPP REQUIRED portaudiocpp)
else()

  find_path(PA_INCLUDE_DIR
    NAMES portaudio.h
  )
  find_path(PACPP_INCLUDE_DIR
    NAMES portaudiocpp/PortAudioCpp.hxx
    PATHS ${PA_INCLUDE_DIR}
  )

  find_library(PA_LIBRARY
    NAMES portaudio
  )

  # dependencies
  # TODO
endif()

find_library(PACPP_LIBRARY
  NAMES portaudiocpp
  PATHS ${PACPP_LINK_LIBRARIES}
)

find_package_handle_standard_args(portaudio::portaudiocpp
  REQUIRED_VARS PACPP_LIBRARY
                # PACPP_INCLUDE_DIR
                PACPP_LINK_LIBRARIES
)

if (${USE_STATIC_LIBS})
  add_library(portaudio::portaudiocpp STATIC IMPORTED GLOBAL)
  set_target_properties(portaudio::portaudiocpp PROPERTIES
    INTERFACE_LINK_LIBRARIES ${PACPP_LINK_LIBRARIES}
    IMPORTED_LOCATION ${PACPP_LIBRARY}
  )


  target_link_libraries(portaudio::portaudiocpp
    INTERFACE portaudio_static
  )
else() # defaults to shared lib
  add_library(portaudio::portaudiocpp SHARED IMPORTED GLOBAL)
  set_target_properties(portaudio::portaudiocpp PROPERTIES
    INTERFACE_LINK_LIBRARIES ${PACPP_LINK_LIBRARIES}
    IMPORTED_LOCATION ${PACPP_LIBRARY}
  )
  target_link_libraries(portaudio::portaudiocpp
    INTERFACE portaudio
  )
endif()

if (PACPP_INCLUDE_DIR)
  set_target_properties(portaudio::portaudiocpp PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${PACPP_INCLUDE_DIR}
)
endif()
