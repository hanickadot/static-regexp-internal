#include <string>
#include <iostream>

#include <static-regexp/static-regexp.hpp>

using namespace sre;

int main(int argc, const char ** argv) {
	
	using Domain = Sequence<Alpha,Star<AlphaNumeric>>;
	using Domains = OneCatch<1,Sequence<Domain,Star<Char<'.'>,Domain>>>;
	
	RegExp<Begin,Alpha,Star<AlphaNumeric>,String<':','/','/'>,Domains,End> http;
	
	if (http.match(argv[1])) {
		CatchRange cr;
		if (http.get<1>(cr)) {
			printf("match: '%.*s'\n",cr[0].len(),argv[1]+cr[0].begin);
		} else {
			printf("match: (no-catch)\n");
		}
		
	} else {
		puts("not match");
	}
	
	return 0;
}
