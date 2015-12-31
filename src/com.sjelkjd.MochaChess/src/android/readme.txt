The Android plugin consists of two parts:
	-java source code in java, under the /java directory
	-a dynamic library written in c++, under the /MochaChessLib directory
The java code links in the library, which is stored in /lib.  The cordova build will not build this library - to modify the code, build the c++ library and copy MochaChessLib.so to /lib.