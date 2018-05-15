all:
	make -C ./build/

remake:
	make -C ./build/ clean
	make -C ./build/

install:
	make -C ./build/ install

clean:
	make -C ./build/ clean

cmake:
	mkdir -p build && cd build && rm -rf * && cmake .. -DCMAKE_TOOLCHAIN_FILE=../ToolChain.cmake
