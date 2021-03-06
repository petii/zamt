# All available modules are enumerated here (they are compiled and tested)

set(zamt_modules
  core
  liveaudio_pulse
  vis_gtk
  dft_fftw
  # vis_vulkan
)


# Find 3rd party libs

find_package(Threads)
find_package(PkgConfig)

# If missing: sudo apt install libpulse-dev
list(FIND zamt_modules liveaudio_pulse liveaudio_pulse_on)
if(liveaudio_pulse_on GREATER -1)
  pkg_check_modules(PulseAudio REQUIRED IMPORTED_TARGET libpulse)
endif()

# If missing: sudo apt install libgtkmm-3.0-dev
list(FIND zamt_modules vis_gtk vis_gtk_on)
if(vis_gtk_on GREATER -1)
  pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET gtkmm-3.0)
endif()

# If missing: sudo apt install libfftw3-dev
list(FIND zamt_modules dft_fftw dft_fftw_on)
if (dft_fftw_on GREATER -1)
  pkg_check_modules(FFTW3 REQUIRED IMPORTED_TARGET fftw3f)
endif()

# If missing:
list(FIND zamt_modules vis_vulkan vis_vulkan_on)
if (vis_vulkan_on GREATER -1)

endif()
