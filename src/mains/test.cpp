#include <string>
#include <iostream>

#include <static-regexp/static-regexp.hpp>

using namespace sre;

int main() {
	
	std::string tmp{"ab"};
	
	if (RegExp<Begin, Range<'a','z'>, Range<'a','z'>, End>::match(tmp)) {
		puts("match");
	} else {
		puts("not match");
	}
	
	return 0;
}
