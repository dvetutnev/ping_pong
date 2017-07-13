# Simple network "Ping-Pong" for lazy student ;)
IO is async, used [libuv](https://github.com/libuv/) and [uvw](https://github.com/skypjack/uvw)
# Build
Is required compiler C++14 

    git clone https://github.com/dvetutnev/ping_pong.git
    cd ping_pong
    git submodule update --init
    mkdir build && cd build
    cmake ..
    cmake --build .
