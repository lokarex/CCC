# Obtain OS type
ifeq ($(shell uname), Linux)
    OS_TYPE = linux
else ifeq ($(shell uname), Darwin)
    OS_TYPE = macos
else ifeq ($(OS), Windows_NT)
    OS_TYPE = windows
else
    OS_TYPE = unknown
	$(error Unsupported OS.)
endif

COMPILER=g++


debug:
	make build COMPILER=$(COMPILER) COMPILE_FLAGS="-g -std=c++20 -W -Wall -Wextra" -B

release:
	make build COMPILER=$(COMPILER) COMPILE_FLAGS="-O2 -std=c++20 -W -Wall -Wextra" -B

build:
	make -f ./packages/cccboot/Makefile COMPILER=$(COMPILER) COMPILE_FLAGS="$(COMPILE_FLAGS)" -j
	make -f ./packages/cccruntime/Makefile COMPILER=$(COMPILER) COMPILE_FLAGS="$(COMPILE_FLAGS)" -j
	make -f ./packages/cccsdk/Makefile COMPILER=$(COMPILER) COMPILE_FLAGS="$(COMPILE_FLAGS)" -j
	mkdir -p ./build/inc
	cp -r ./packages/cccsdk/include/* ./build/inc

clean:
	rm -rf ./build

ifeq ($(OS_TYPE), windows)
TARGET_PATH = C:$(shell cmd /c echo %HOMEPATH%)\.ccc
endif
ifeq ($(OS_TYPE), linux)
TARGET_PATH = $(shell echo $$HOME)/.ccc
endif

install:
	mkdir -p $(TARGET_PATH)
	cp -r ./build $(TARGET_PATH)
ifeq ($(OS_TYPE), windows)
	python ./script/windows_installer.py --target_path=$(TARGET_PATH)
endif
ifeq ($(OS_TYPE), linux)
	python ./script/linux_installer.py --target_path=$(TARGET_PATH) > /dev/null 2>&1 || python3 ./script/linux_installer.py --target_path=$(TARGET_PATH)
endif

uninstall:
	rm -rf $(shell echo $$HOME)/.ccc/build
