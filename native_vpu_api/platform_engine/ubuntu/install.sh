#!/bin/bash

if [ -z $1 ];then
    echo "Please input cmd as (sudo ./install [<install path>]). "
    exit
fi

project_path=$(cd `dirname $0`;pwd)
install_path=/usr/lib/openncc

library=$project_path/libOpenNCC_native.so
boot_tool=$project_path/moviUsbBoot

cp $library $1
cp $boot_tool $1

echo "Installing $library to $1"
echo "Installing $boot_tool to $1"
