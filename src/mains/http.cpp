#include <string>
#include <iostream>

#include <static-regexp/static-regexp.hpp>

using namespace sre;

int main(int argc, const char ** argv) {
	
	using Domain = Sequence<Alpha,Star<AlphaNumeric>>;
	using Domains = OneCatch<1,Sequence<Domain,Star<Char<'.'>,Domain>>>;
	
	RegExp<Begin,Plus<AlphaNumeric>,String<':','/','/'>,Domains,Char<'/'>> http;
	
	if (http.match(argv[1])) {
		CatchRange cr{http.get<1>()};
		if (cr) {
			printf("match: '%.*s'\n",cr[0].len(),argv[1]+cr[0].begin);
		} else {
			puts("match");
		}
		
		//CatchRange cr;
		//if (http.get<1>(cr)) {
		//	printf("match: '%.*s'\n",cr[0].len(),argv[1]+cr[0].begin);
		//} else {
		//	printf("match: (no-catch)\n");
		//}
		
	} else {
		puts("no");
	}
	
	return 0;
}
