## Debuild, build with system libraries. (Ubuntu/Debian)
### Prepare for debuild (installing required dependencies).

(TODO:improve it) To build the .deb packet, we need firstly install the nnstreamer. Since nccframe filters depends on nnstreamer

```shell
$ sudo add-apt-repository ppa:nnstreamer/ppa
$ sudo apt-get update
$ sudo apt-get install libprotobuf-dev
$ git clone https://github.com/nnstreamer/nnstreamer
$ cd nnstreamer
$ meson build --prefix=/usr --sysconfdir=/etc --libdir=/usr/lib/x86_64-linux-gnu --bindir=/usr/bin --includedir=/usr/include
$ sudo ninja -C build install
```

You need on the root path of the opennccframe project 

```shell
$ sudo apt-get install devscripts build-essential equivs
$ sudo mk-build-deps --install debian/control  
$ sudo dpkg -i *.deb  
```

Need to install ncc native sdk  

```shell
$ sudo ./install.sh ubuntu
```

### Update the changelog, add new version(Don't remove older ones)

```shell
$ gedit debian/changelog
```

### Run debuild and get .deb packages.

```shell
$ export DEB_BUILD_OPTIONS="parallel=$(($(cat /proc/cpuinfo |grep processor|wc -l) + 1))"  

# The following step would automatic download nnstreamer from github. If you already have it, you could copy it to openncc_frame, and rename it to nnstmp. That will skip re-download the repo. 
$ time debuild -us -uc  
 
```

### check .deb

```shell
$ dpkg-deb -c <packageName>
$ dpkg -l | grep nnstreamer

# clean
$ sudo dpkg -r nnstreamer
$ sudo add-apt-repository --remove ppa:nnstreamer/ppa apt show libprotobuf25
```

### Install the generated .deb files.

```shell
$ sudo dpkg -i <opennccframe_*.*.*_amd64.deb>
```