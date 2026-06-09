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

# Obtain source files
SOURCE_FILES = $(wildcard ./packages/cccsdk/src/ccc/*.cpp) \
               $(wildcard ./packages/cccsdk/src/util/*.cpp)

# Set object files
ifeq ($(OS_TYPE), windows)
OBJECT_FILES = $(patsubst ./packages/cccsdk/src/%.cpp,./build/obj/cccsdk/src/%.obj,$(SOURCE_FILES))
else ifeq ($(OS_TYPE), linux)
OBJECT_FILES = $(patsubst ./packages/cccsdk/src/%.cpp,./build/obj/cccsdk/src/%.o,$(SOURCE_FILES))
endif


# Link
ifeq ($(OS_TYPE), windows)
./build/lib/cccsdk.dll: $(OBJECT_FILES)
else ifeq ($(OS_TYPE), linux)
./build/lib/libcccsdk.so: $(OBJECT_FILES)
endif
	mkdir -p ./build/lib
ifeq ($(OS_TYPE), windows)
	g++ -shared -o ./build/lib/cccsdk.dll $(OBJECT_FILES)
else ifeq ($(OS_TYPE), linux)
	g++ -shared -o ./build/lib/libcccsdk.so $(OBJECT_FILES)
endif

# Compile
ifeq ($(OS_TYPE), windows)
./build/obj/cccsdk/src/%.obj: ./packages/cccsdk/src/%.cpp
else ifeq ($(OS_TYPE), linux)
./build/obj/cccsdk/src/%.o: ./packages/cccsdk/src/%.cpp
endif
	mkdir -p $(dir $@)
	$(COMPILER) -c $< -o $@  -I./packages/cccsdk/include $(COMPILE_FLAGS) -fPIC
