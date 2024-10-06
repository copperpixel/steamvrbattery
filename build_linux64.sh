mkdir -p out
rm -r out/*

g++ -o out/steamvrbattery steamvrbattery.cpp -Iinclude/ -Llib/linux64 -lopenvr_api

cp bin/linux64/libopenvr_api.so out/

