all:
	make -C ./build/

remake:
	make -C ./build/ clean
	make -C ./build/

install:
	make -C ./build/ install

clean:
	make -C ./build/ clean

