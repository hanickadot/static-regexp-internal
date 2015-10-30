#include <string>
#include <iostream>
#include <cassert>
#include <static-regexp/static-regexp.hpp>

using namespace sre;

bool checkCycleString(const std::string & str) {
	return RegExp<Begin, Repeat<3,5,Star<Range<'0','9'>>,Range<'a','z'>>, Select<End,Char<'_'>>>{}.match(str);
}

bool checkMemory(const std::string & str, unsigned int count) {
	using Content = StaticCatch<1,10,Plus<Range<'a','z'>,StaticCatch<2,10,Star<Range<'a','z'>>> > >;
	RegExp<Begin, Plus< Content ,Char<'.'>>, End> re;
	
	if (re.match(str)) {
		std::cout << "Input: '"<<str<<"'\n";
		CatchRange cr;
		size_t got{re.get<1>(cr)};
		for (auto & pair: cr) {
			std::cout << "1:"<< pair(str) << "\n";
		}
		std::cout << "\n";
		re.get<2>(cr);
		for (auto & pair: cr) {
			std::cout << "2:"<< pair(str) << "\n";
		}
		std::cout << "\n";
		assert(count == got);
		return true;
	}
	return false;
}

int main(int argc, char ** argv) {
	assert(checkCycleString("") == false);
	assert(checkCycleString("1a") == false);
	assert(checkCycleString("1a2b") == false);
	assert(checkCycleString("1a2b3c") == true);
	assert(checkCycleString("1a2b3c4d") == true);
	assert(checkCycleString("1a2b3c4d5e") == true);
	assert(checkCycleString("1a2b3c4d5e6f") == false);
	assert(checkCycleString("1a2b3c4d5e_6f") == true);
	assert(checkCycleString("1a2b3333c4d5e") == true);
	assert(checkCycleString("1a2bc4d5e") == true);
	
	assert(checkMemory("aaazz.bbbzz.ccczz.",3) == true);
	assert(checkMemory("aaa.",1) == true);
	assert(checkMemory("",0) == false);
	
	return 0;
}
