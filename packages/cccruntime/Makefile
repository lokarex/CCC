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
	mkdir -p ./build/obj/packages/cccruntime/src
ifeq ($(OS_TYPE), windows)
	$(COMPILER) ./packages/cccruntime/src/cccruntime.cpp -o ./build/obj/packages/cccruntime/src/cccruntime.obj -c -I./packages/cccsdk/include $(COMPILE_FLAGS)
else ifeq ($(OS_TYPE), linux)
	$(COMPILER) ./packages/cccruntime/src/cccruntime.cpp -o ./build/obj/packages/cccruntime/src/cccruntime.o -c -I./packages/cccsdk/include $(COMPILE_FLAGS)
endif

ifeq ($(OS_TYPE), windows)
	$(ARCHIVER) ./build/lib/libcccruntime.lib ./build/obj/packages/cccruntime/src/cccruntime.obj
else ifeq ($(OS_TYPE), linux)
	$(ARCHIVER) ./build/lib/libcccruntime.a ./build/obj/packages/cccruntime/src/cccruntime.o
endif
