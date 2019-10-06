# All target apps are configured here (what modules they are built of...)

set(modules
  core
  liveaudio_pulse
  vis_gtk
  vis_vulkan
)
AddExe(zamtdemo "${modules}")
set_property(TARGET zamtdemo PROPERTY CXX_STANDARD 17)
