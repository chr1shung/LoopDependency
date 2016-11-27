LLVM_CONFIG=llvm-config

#CXX=g++
CXX=`$(LLVM_CONFIG) --bindir`/clang
CXXFLAGS=`$(LLVM_CONFIG) --cppflags` -std=gnu++11 -fPIC -fno-rtti -g
LDFLAGS=`$(LLVM_CONFIG) --ldflags`

all: HW.so

HW.so: HW.o
	$(CXX) -shared HW.o -o HW.so  -fPIC

HW.o: HW.cpp
	$(CXX) -c HW.cpp -o HW.o $(CXXFLAGS)

clean:
	rm -f *.o *.so
