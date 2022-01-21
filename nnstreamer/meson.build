project('nnstreamer', 'cpp',
  version: '1.0.0',
  license: ['Proprietary'],
  meson_version: '>=0.50.0',
)

cxx = meson.get_compiler('cpp')

gst_api_verision = '1.0'

# Set install path
nnstreamer_prefix = get_option('prefix')
nnstreamer_libdir = join_paths(nnstreamer_prefix, get_option('libdir'))
nnstreamer_bindir = join_paths(nnstreamer_prefix, get_option('bindir'))
nnstreamer_includedir = join_paths(nnstreamer_prefix, get_option('includedir'))
nnstreamer_inidir = join_paths(nnstreamer_prefix, get_option('sysconfdir'))
# join_paths drops first arg if second arg is absolute path.

# nnstreamer plugins path
plugins_install_dir = join_paths(nnstreamer_libdir, 'gstreamer-' + gst_api_verision)

# nnstreamer sub-plugins path
if get_option('subplugindir') == ''
  subplugin_install_prefix = join_paths(nnstreamer_prefix, get_option('libdir'), 'nnstreamer')
else
  subplugin_install_prefix = get_option('subplugindir')
endif
filter_subplugin_install_dir = join_paths(subplugin_install_prefix, 'filters')
decoder_subplugin_install_dir = join_paths(subplugin_install_prefix, 'decoders')
customfilter_install_dir = join_paths(subplugin_install_prefix, 'customfilters')
converter_subplugin_install_dir = join_paths(subplugin_install_prefix, 'converters')
extra_install_dir = join_paths(subplugin_install_prefix, 'extra')
unittest_base_dir = join_paths(nnstreamer_bindir, 'unittest-nnstreamer')

# Register plugins information
ncc_conf = configuration_data()
ncc_conf.set('PACKAGE_VERSION', '"1.0"')
ncc_conf.set('PACKAGE', '"gst-template-plugin"')
ncc_conf.set('GST_LICENSE', '"LGPL"')
ncc_conf.set('GST_API_VERSION', '"1.0"')
ncc_conf.set('GST_PACKAGE_NAME', '"GStreamer template Plug-ins"')
ncc_conf.set('GST_PACKAGE_ORIGIN', '"https://gstreamer.freedesktop.org"')
configure_file(output : 'config.h', configuration : ncc_conf)
configinc = include_directories('.')

# Dependencies
glib_dep = dependency('glib-2.0')
if glib_dep.version().version_compare('>= 2.68.0')
  add_project_arguments('-DGLIB_USE_G_MEMDUP2', language: ['c', 'cpp'])
endif
gobject_dep = dependency('gobject-2.0')
gmodule_dep = dependency('gmodule-2.0')
gio_dep = dependency('gio-2.0')
gst_dep = dependency('gstreamer-' + gst_api_verision)
gst_base_dep = dependency('gstreamer-base-' + gst_api_verision)
gst_controller_dep = dependency('gstreamer-controller-' + gst_api_verision)
gst_video_dep = dependency('gstreamer-video-' + gst_api_verision)
gst_audio_dep = dependency('gstreamer-audio-' + gst_api_verision)
gst_app_dep = dependency('gstreamer-app-' + gst_api_verision)
gst_check_dep = dependency('gstreamer-check-' + gst_api_verision)

libusb_dep = dependency('libusb-1.0')
gstvideo_dep = dependency('gstreamer-video-1.0')
gstallocators_dep = dependency('gstreamer-allocators-1.0')

nnstreamer_dep = dependency('nnstreamer')
ncc_lib_dir = '/usr/local/lib/openncc'
ncc_dep = cxx.find_library('OpenNCC_native', dirs : ncc_lib_dir)

# Build nnstreamer (common, plugins)
subdir('gst')
