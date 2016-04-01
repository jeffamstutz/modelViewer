#/bin/sh

ROOT_DIR=${PWD}

# Clone and build ospray if we don't have one yet
if [ ! -d ospray ]; then
  echo 'OSPRay build does not exist!'
  echo 'Building OSPRay from Github:'
  # Create directory
  mkdir ospray
  cd ospray
  # Clone source
  echo -n 'cloning source...'
  git clone https://github.com/ospray/ospray.git src &> /dev/null
  echo 'done'
  # Checkout 'devel' branch
  echo -n 'checking out devel branch...'
  cd src
  git checkout devel &> /dev/null
  cd ..
  echo 'done'
  # Create a build
  echo -n 'compiling OSPRay...'
  mkdir build
  cd build
  cmake \
    -DCMAKE_INSTALL_PREFIX=../install\
    ../src &> /dev/null
  make -j`nproc` install &> /dev/null
  echo 'done'
else
  echo 'Updating exisiting OSPRay build:'
  cd ospray/src
  echo -n 'pulling latest devel branch ref...'
  git pull origin &> /dev/null
  echo 'done'
  echo -n 'compiling OSPRay...'
  cd ../build
  make -j`nproc` install &> /dev/null
  echo 'done'
fi

echo ' '
echo 'OSPRay build is up to date!...moving on to ospDebugViewer'
echo ' '

# Set variables we need to find OSPRay
export ospray_DIR=${ROOT_DIR}/ospray/install

# Make sure we return to our original clone directory
cd ${ROOT_DIR}

# Build ospDebugViewer project
mkdir build
cd build
rm -rf *
cmake ..
make -j`nproc`

