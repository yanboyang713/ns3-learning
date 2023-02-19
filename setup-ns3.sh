#!/bin/bash
current_dir=$(pwd)

# ns3 version 3.37
ns3_ver="3.37"
ns3_folder="ns-allinone-${ns3_ver}"
ns3_scratch="${current_dir}/${ns3_folder}/ns-${ns3_ver}/scratch"
ns3_src="${current_dir}/${ns3_folder}/ns-${ns3_ver}/src"

# download ns3 to current dir
if [ ! -d "${current_dir}"/${ns3_folder} ]; then
    ns3_file_name="ns-allinone-${ns3_ver}.tar.bz2"
    url="https://www.nsnam.org/releases/${ns3_file_name}"
    if [ ! -f "${current_dir}/${ns3_file_name}" ]; then
        echo "Downloading NS3-${ns3_ver}..."
        wget --show-progress --quiet $url
    fi
    # unzip
    echo "Unzipping ${ns3_file_name}..."
    tar xjf ${ns3_file_name}
    # Delete tar file
    echo "Deleteing TAR file"
    rm ${ns3_file_name}

fi

# compile (opitonal)
echo "Compiling ns3..."
cd "${current_dir}"/${ns3_folder} || return
./build.py

# link scratch in assets
echo "Copying scratch to ns3..."
ln -s -v "${current_dir}"/scratch/*.cc "${ns3_scratch}"/

# link src in assets
echo "Copying src to ns3..."
ln -s -v "${current_dir}"/src/* "${ns3_src}"/
