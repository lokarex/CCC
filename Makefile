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

TARGET_PATH = $(shell echo $$HOME)/.ccc

install:
	mkdir -p $(TARGET_PATH)
	cp -r ./build $(TARGET_PATH)
	cd scripts && python installer.py --target_path=$(TARGET_PATH)

uninstall:
	cd scripts && python uninstaller.py
	rm -rf $(TARGET_PATH)
