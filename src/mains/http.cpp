#include <string>
#include <iostream>

#include <static-regexp/static-regexp.hpp>

using namespace sre;

int main(int argc, const char ** argv) {
	
	using Domain = Sequence<Alpha,Star<AlphaNumeric>>;
	using Domains = OneCatch<1,Sequence<Domain,Star<Char<'.'>,Domain>>>;
	
	RegExp<Begin,Alpha,Star<AlphaNumeric>,String<':','/','/'>,Domains,End> http;
	
	if (http.match(std::string(argv[1]))) {
		CatchRange cr;
		if (http.get<1>(cr)) {
			printf("match: '%s'\n",NULL);
		} else {
			printf("match: (no-catch)\n");
		}
		
	} else {
		puts("not match");
	}
	
	return 0;
}
