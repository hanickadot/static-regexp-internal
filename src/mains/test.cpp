#include <string>
#include <iostream>

#include <static-regexp/static-regexp.hpp>

using namespace sre;

int main(int argc, char ** argv) {
	
	std::string tmp{argv[1]};
	
	if (RegExp<Begin, Select<Sequence<Range<'a','z'>, Range<'0','9'>>, Range<'a','z'>>, End>::match(tmp)) {
		puts("match");
	} else {
		puts("not match");
	}
	
	return 0;
}
