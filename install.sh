#!/bin/bash

if [ -z $1 ];then
    echo "Please input cmd as (sudo ./install [<platform>]). "
    echo "[<platform>]: [ubuntu][rk3568][raspi4]"
    exit
fi

if [ -z $2 ];then
    echo "Please input cmd as (sudo ./install [<platform>][<dest install dir>]). "
    echo "defalut dir:/usr"
    destdir=/usr
else
    destdir=$2
fi

multiarch=$(dpkg-architecture -qDEB_HOST_MULTIARCH)
project_path=$(cd `dirname $0`;pwd)
install_pkg_path=$destdir/lib/pkgconfig
install_path=$destdir/lib/openncc
include_path=$destdir/include/openncc

native_lib_path=$project_path/native_vpu_api
resource_path=$native_lib_path/platform_engine
firmware_path=$native_lib_path/firmware
model_path=$project_path/model_zoo

if [ ! -d "$install_path" ];then
    mkdir $install_path
else
    rm -r $install_path
    mkdir $install_path
fi

if [ ! -d "$include_path" ];then
    mkdir $include_path
else
    rm -r $include_path
    mkdir $include_path
fi

if [ $1 = "ubuntu"  ] || [ $1 = "rk3568" ] || [ $1 = "raspi4" ];then 
    echo "Starting to install OpenNCC Native libs for $1"
    cd $resource_path/$1/
    ./install.sh $install_path
else
    echo "Unknown platform!"
fi

cp $firmware_path/* $install_path
cp $resource_path/include/* $include_path
cp -r $model_path $install_path
if [ ! -d "$install_pkg_path" ];then
	mkdir $install_pkg_path
fi	
#cp -f $native_lib_path/OpenNCC_native.pc  $install_pkg_path/

echo "Installing $firmware_path to $install_path"
echo "Installing $resource_path/include to $include_path"
echo "Installing $model_path to $install_path"

