#!/bin/sh

data_dir="test_data"

# Create data directory and fill it
if [ ! -d ${data_dir} ]; then
  mkdir ${data_dir}
fi

cd ${data_dir}

# FIU
fiu="fiu.bz2"
if [ ! -e ${fiu} ]; then
  wget http://sdvis.org/~jdamstut/test_data/${fiu}
  tar -xaf ${fiu}
fi

# HEPTANE
csafe="csafe.bz2"
if [ ! -e ${csafe} ]; then
  wget http://sdvis.org/~jdamstut/test_data/${csafe}
  tar -xaf ${csafe}
fi
