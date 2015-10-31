#include <string>
#include <iostream>

#include <static-regexp/static-regexp.hpp>

using namespace sre;

int main(int argc, const char ** argv) {
	
	FloatingRegExp<String<'h','a','n','a'>,End> hana;
	
	if (hana.match(argv[1])) {
		puts("yes");
	} else {
		puts("no");
	}
	
	return 0;
}
