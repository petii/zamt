# All target apps are configured here (what modules they are built of...)

set(modules
  core
  liveaudio_pulse
  vis_gtk
)
AddExe(zamtdemo "${modules}")

set(vis_vulk_modules
  vis_vulkan
)
AddExe(vis_vulk "${vis_vulk_modules}")
set_property(TARGET vis_vulk PROPERTY CXX_STANDARD 17)
