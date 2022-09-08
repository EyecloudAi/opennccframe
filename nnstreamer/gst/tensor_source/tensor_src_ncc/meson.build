plugin_cpp_args = ['-DHAVE_CONFIG_H']
base_deps = [
  glib_dep,
  gmodule_dep,
  gst_dep,
  nnstreamer_dep,
  gst_base_dep,
  libusb_dep,
  ncc_dep
]

incdir = include_directories('/usr/include/openncc')

sources = [
  'tensor_src_ncc.cpp'
]

subplugin_shared = shared_library('ncctensorsrc',
  sources,
  cpp_args: plugin_cpp_args,
  include_directories : [configinc, incdir],
  dependencies: base_deps,
  link_args:['-lpthread'],
  install: true,
  install_dir: plugins_install_dir
)
