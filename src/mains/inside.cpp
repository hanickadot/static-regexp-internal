#include <string>
#include <iostream>

#include <static-regexp/static-regexp.hpp>

using namespace sre;

int main(int argc, const char ** argv) {
	
	FloatingRegExp<String<'h','a','n','a'>,End> hana;
	
	printf("%zu\n",sizeof(hana));
	
	if (hana.match("muhaha hana")) {
		puts("yes");
	} else {
		puts("no");
	}
	
	return 0;
}
