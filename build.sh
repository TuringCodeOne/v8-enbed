g++ -I./v8/include src/main.cc -o out/hello_world -lv8_monolith -Llib -pthread -std=c++14 -DV8_COMPRESS_POINTERS -ldl
