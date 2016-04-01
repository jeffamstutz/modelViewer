#/bin/sh

ROOT_DIR=${PWD}
CORNELL_BOX_TARBALL=cornell_box.bz2

# If build doesn't exist, run the build again
if [ ! -d build ]; then
  echo -n 'build does not exist, making a new one...'
  ${ROOT_DIR}/scripts/build_linux.sh &> /dev/null
  echo 'done'
fi

# Fetch test data
mkdir test_data
cd test_data
echo 'fetching test data...'
wget http://sdvis.org/~jdamstut/test_data/${CORNELL_BOX_TARBALL}
echo 'extracting test data...'
tar -xaf ${CORNELL_BOX_TARBALL}
rm ${CORNELL_BOX_TARBALL}

# Go to the build directory
cd ${ROOT_DIR}/build

# Do a cornell_box benchmark run
echo 'run benchmark:'
./ospBenchmark \
  ${ROOT_DIR}/test_data/cornell_box.obj \
  -vp 283.271912 390.423401 -536.325806 \
  -vu 0.000000 1.000000 0.000000 \
  -vi 278.000000 274.399994 279.599976 \
  --renderer ao1

