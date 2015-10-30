#include <string>
#include <iostream>

#include <static-regexp/static-regexp.hpp>

using namespace sre;

int main(int argc, char ** argv) {
	
	std::string tmp{argv[1]};
	
	using Bad = RegExp<Begin, Repeat<3,5,Star<Range<'0','9'>>,Range<'a','z'>>, Select<End,Char<'_'>>>;
	using Good = RegExp<Begin, Repeat<3,5,Range<'0','9'>,Range<'a','z'>>, Select<End,Char<'_'>>>;
	
	if (Bad::match(tmp)) {
		puts("match");
	} else {
		puts("not match");
	}
	
	return 0;
}
