BUFFERSIZE = 65536

default: splice.h splice.cpp spliceDemo.cpp
	clang++ -v -framework CoreServices -DBUFFERSIZE=$(BUFFERSIZE) spliceDemo.cpp splice.cpp -o demo -x c ed25519/*.c libb64/*.c

clean:
	rm demo