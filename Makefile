zimcompare: zimcompare.cpp
	g++ -o zimcompare zimcompare.cpp  /usr/local/lib/libzim.a -llzma -std=c++0x
