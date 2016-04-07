#!/bin/sh

exe="./ospBenchmark"
img_dir="test_images"

# Create image directory
if [ ! -d ${img_dir} ]; then
  mkdir ${img_dir}
fi

# Test counter
test_num=0

# FIU TESTS #################################################################

fiu="test_data/fiu-groundwater.xml"

# FIU/scivis/view 1
${exe} ${fiu} \
    -vp 500.804565 277.327850 -529.199829 \
    -vu 0.000000 1.000000 0.000000 \
    -vi 21.162066 -62.059830 -559.833313 \
    -r scivis \
    -i ${img_dir}/test_${test_num}

test_num=$((test_num+1))

# FIU/scivis/view 2
${exe} ${fiu} \
    -vp -29.490566 80.756294 -526.728516 \
    -vu 0.000000 1.000000 0.000000 \
    -vi 21.111689 12.973234 -443.164886 \
    -r scivis \
    -i ${img_dir}/test_${test_num}

test_num=$((test_num+1))

# HEPTANE TESTS ##############################################################

csafe="test_data/csafe-heptane-302-volume.osp"

# CSAFE/scivis/view 1
${exe} ${csafe} \
    -r scivis \
    -tfc 0 0 0 0.00 \
    -tfc 1 0 0 0.11 \
    -tfc 1 1 0 0.22 \
    -tfc 1 1 1 0.33 \
    -tfs 0.25 \
    -i ${img_dir}/test_${test_num}

test_num=$((test_num+1))

# CSAFE/scivis/view 2
${exe} ${csafe} \
    -r scivis \
    -vp -36.2362 86.8541 230.026 \
    -vi 150.5 150.5 150.5 \
    -vu 0 0 1 \
    -tfc 0 0 0 0.00 \
    -tfc 1 0 0 0.11 \
    -tfc 1 1 0 0.22 \
    -tfc 1 1 1 0.33 \
    -tfs 0.25 \
    -i ${img_dir}/test_${test_num}

test_num=$((test_num+1))

# MAGNETIC TESTS ############################################################

magnetic="test_data/magnetic-512-volume.osp"

# MAGNETIC/scivis/view 1
${exe} ${magnetic} \
    -r scivis \
    -tfc 0.1 0.1 0.1 0 \
    -tfc 0 0 1 0.25 \
    -tfc 1 0 0 1 \
    -tfs 1 \
    -i ${img_dir}/test_${test_num}

test_num=$((test_num+1))

# MAGNETIC/scivis/view 2
${exe} ${magnetic} \
    -r scivis \
    -vp 431.923 -99.5843 408.068 \
    -vi 255.5 255.5 255.5 \
    -vu 0 0 1 \
    -tfc 0.1 0.1 0.1 0 \
    -tfc 0 0 1 0.25 \
    -tfc 1 0 0 1 \
    -tfs 1 \
    -i ${img_dir}/test_${test_num}

# MAGNETIC/scivis/view 2 + isosurface
${exe} ${magnetic} \
    -r scivis \
    -vp 431.923 -99.5843 408.068 \
    -vi 255.5 255.5 255.5 \
    -vu 0 0 1 \
    -tfc 0.1 0.1 0.1 0 \
    -tfc 0 0 1 0.25 \
    -tfc 1 0 0 1 \
    -tfs 1 \
    -is 2.0 \
    -i ${img_dir}/test_${test_num}

test_num=$((test_num+1))

