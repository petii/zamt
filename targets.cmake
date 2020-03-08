# All target apps are configured here (what modules they are built of...)

include(AddTarget)
include(AddTest)

set(modules
  core
  liveaudio_pulse
  vis_gtk
  dft_fftw
)
AddExe(zamtdemo "${modules}")

# and what/where is zamt_modules?
AddAllTests("${zamt_modules}")

