# PRS - CTP - Loïc Bourgois

## Mark
14/20

## Install
```bash
sudo apt-get install gcc
mkdir build
```

## Build
```bash
gcc -pthread -Wall ./sources/ctp.c -o ./build/ctp
gcc -pthread -Wall ./sources/ctp-v2.c -o ./build/ctp-v2
```

## Run
```bash
./build/ctp
./build/ctp-v2
```
