export $(cat .env | xargs)
rm -rf build
mkdir build
cd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DPICO_SDK_PATH="/home/timoha/Projects/rpi/pico-sdk" -DPICO_BOARD=pico2_w ..
