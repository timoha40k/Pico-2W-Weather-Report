set -a
source .env
set +a

cd build
make -j4
