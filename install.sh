#!/bin/bash

if [ -z $1 ];then
    echo "Please input cmd as (sudo ./install [<platform>]). "
    echo "[<platform>]: [ubuntu]"
    exit
fi

project_path=$(cd `dirname $0`;pwd)
install_path=/usr/local/lib/openncc
include_path=/usr/local/include/openncc

resource_path=$project_path/native_vpu_api/platform_engine
firmware_path=$project_path/native_vpu_api/firmware
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

if [ $1 = "ubuntu" ];then 
    cd $resource_path/$1/
    ./install.sh $install_path
else
    echo "Unknown platform!"
fi

cp $firmware_path/* $install_path
cp $resource_path/include/* $include_path
cp -r $model_path $install_path

echo "Installing $firmware_path to $install_path"
echo "Installing $resource_path/include to $include_path"
echo "Installing $model_path to $install_path"

