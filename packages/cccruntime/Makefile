# Obtain OS type
ifeq ($(shell uname), Linux)
    OS_TYPE = linux
else ifeq ($(shell uname), Darwin)
    OS_TYPE = macos
else ifeq ($(OS), Windows_NT)
    OS_TYPE = windows
else
    OS_TYPE = unknown
endif

# Compile
ifeq ($(OS_TYPE), windows)
./build/lib/libcccruntime.lib: ./packages/cccruntime/src/cccruntime.cpp
else ifeq ($(OS_TYPE), linux)
./build/lib/libcccruntime.a: ./packages/cccruntime/src/cccruntime.cpp
endif
	mkdir -p ./build/lib
ifeq ($(OS_TYPE), windows)
	$(COMPILER) ./packages/cccruntime/src/cccruntime.cpp -o ./build/lib/libcccruntime.lib -c -I./packages/cccsdk/include $(COMPILE_FLAGS)
else ifeq ($(OS_TYPE), linux)
	$(COMPILER) ./packages/cccruntime/src/cccruntime.cpp -o ./build/lib/libcccruntime.a -c -I./packages/cccsdk/include $(COMPILE_FLAGS)
endif
