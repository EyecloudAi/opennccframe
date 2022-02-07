base_deps = [
  glib_dep,
  gmodule_dep,
  gst_dep,
  nnstreamer_dep,
  libusb_dep,
  ncc_dep
]

incdir = include_directories('/usr/include/openncc')

sources = [
  'tensor_filter_ncc.cpp'
]

subplugin_shared = shared_library('nnstreamer_filter_ncc',
  sources,
  include_directories : incdir,
  dependencies: base_deps,
  install: true,
  install_dir: filter_subplugin_install_dir
)
