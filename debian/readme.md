## Debuild, build with system libraries. (Ubuntu/Debian)
### Clean the environment,these would be installed from nnstreamer ppa,but they are not supported on arm linux by nnstreamer,so we need disable them
$sudo apt-get autoremove libflatbuffers libpaho-mqtt1.3 libprotobuf25  
###(TODO:improve it) To build the .deb packet,we need firstly install the nnstream.Since nccframe filters depands on nnstreams
$ sudo add-apt-repository ppa:nnstreamer/ppa
$ sudo apt-get update
$sudo apt-get install libprotobuf-dev
$git clone https://github.com/nnstreamer/nnstreamer
$cd nnstreamer
$meson build --prefix=/ --sysconfdir=/etc --libdir=/usr/lib --bindir=/usr/bin --includedir=/usr/include
$ninja -C build install
### Prepare for debuild (installing required dependencies).
You need on the root path of the opennccframe project  
$sudo apt-get install devscripts build-essential
$sudo mk-build-deps --install debian/control  
$sudo dpkg -i *.deb  
Need to install ncc native sdk  
$sudo ./install.sh ubuntu
### update the changelog,add new version(Don't remove older ones)
$gedit debian/changelog

### Run debuild and get .deb packages.
$export DEB_BUILD_OPTIONS="parallel=$(($(cat /proc/cpuinfo |grep processor|wc -l) + 1))"  
$cd debian
$time debuild -us -uc  
#In the process,it would automatic download nnstreamer from github.If you already have it,you clould copy it to opennccframe,and rename it to nnstmp.That will skipp re-download the repo.  
### Install the generated .deb files.
$sudo apt install ./opennccframe_22.01.01_amd64.deb  

### check .deb
dpkg-deb -c  packageName


dpkg -l | grep nnstreamer
sudo dpkg -r nnstreamer
sudo add-apt-repository --remove ppa:nnstreamer/ppa
apt show libprotobuf25

