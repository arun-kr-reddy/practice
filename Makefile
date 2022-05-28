CXX = g++
CXXFLAGS = -g -O3

test_exe: test.o		# test.o compiled implicitly
	$(CXX) test.o -o test_exe

test.cpp:
	echo "file not found"

clean:
	rm *.o test_exe