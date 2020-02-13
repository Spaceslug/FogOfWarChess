#include <string>
#include <iostream>

#ifdef WIN

//#include <SDKDDKVer.h>

//#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
//#include <windows.h>

#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>

#endif

#include "test.h"


int main(const int argc, const char ** argv)
{		
	std::cout << "Start version " << version << std::endl << std::flush;
	if(wers__initialize("nullptr")){
		std::cout << "wers found " << std::endl << std::flush;
	}else{
		std::cout << "wers not found " << std::endl << std::flush;
	}
	int* array = new int[10];
	int* retArr = CountDownHandleGiven(10, array);
	for(int i = 0; i < 10;i++)
	{
		std::cout << " " << std::to_string(retArr[i]);
	}
	std::cout << ";" << std::endl << std::flush;

	delete[] array;
	return 0;
		
}