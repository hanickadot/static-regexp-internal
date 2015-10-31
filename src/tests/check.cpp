#include <string>
#include <iostream>
#include <cassert>
#include <static-regexp/static-regexp.hpp>

using namespace sre;

bool checkCycleString(const std::string & str) {
	return RegExp<Begin, Repeat<3,5,Star<Range<'0','9'>>,Range<'a','z'>>, Select<End,Char<'_'>>>{}.match(str);
}

template <unsigned int id> using ABC = StaticCatch<id,10,Plus<Range<'a','z'>>>;
template <unsigned int id> using ABCs = Sequence<Plus<Range<'a','z'>>>;
template <unsigned int id> using ABCx = Plus<Range<'a','z'>>;

bool checkEquivalency(const std::string & str) {
	RegExp<Begin, Sequence<ABC<1>,ABC<1>>, End> re1;
	RegExp<Begin, Sequence<ABCs<1>,ABCs<1>>, End> re2;
	RegExp<Begin, Sequence<ABCx<1>,ABCx<1>>, End> re3;
	
	assert(re1.match(str) == re2.match(str));
	assert(re2.match(str) == re3.match(str));
	assert(re1.match(str) == re3.match(str));
	return true;
}

bool checkUnique(const std::string & str, const std::string & left, const std::string & right) {
	RegExp<Begin, ABC<1>,ABC<2>, End> re;
	if (re.match(str)) {
		CatchRange cr;
		assert(re.getRef<1>(cr) == 1);
		for (auto & pair: cr) { assert(pair(str).toString() == left); }
		assert(re.getRef<2>(cr) == 1);
		for (auto & pair: cr) { assert(pair(str).toString() == right); }
		return true;
	}
	return false;
}

bool checkMemory(const std::string & str, unsigned int count, std::initializer_list<std::string> && left, std::initializer_list<std::string> && right) {
	
	using Content = Sequence<ABC<1>,ABC<2>>;
	using ContentX = Sequence<ABCx<1>,ABCx<2>>;
	using ContentS = Sequence<ABCs<1>,ABCs<2>>;
	
	
	RegExp<Begin, Plus< Content ,Char<'.'>>, End> re;
	RegExp<Begin, Plus< ContentS ,Char<'.'>>, End> rex;
	
	assert(rex.match(str) == re.match(str));
	
	if (re.match(str)) {
		CatchRange cr;
		assert(re.getRef<1>(cr) == left.size());
		auto a = left.begin();
		for (auto & pair: cr) {
			//std::cout << "'"<<pair(str).toString()<<"' vs '" << *a << "'\n";
			assert(pair(str).toString() == *a++);
		}
		assert(re.getRef<2>(cr) == right.size());
		auto b = right.begin();
		for (auto & pair: cr) {
			//std::cout << "'"<<pair(str).toString()<<"' vs '" << *b << "'\n";
			assert(pair(str).toString() == *b++);
		}
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
	
	assert(checkEquivalency("aaaxzz") == true);
	
	
	assert(checkMemory("aaaxzz.bbxbzy.cccxzx.",3,{"aaaxz","bbxbz","cccxz"},{"z","y","x"}) == true);
	assert(checkMemory("aaa.",1,{"aa"},{"a"}) == true);
	assert(checkMemory("",0,{},{}) == false);
	assert(checkUnique("abcdef","abcde","f") == true);
	
	return 0;
}
