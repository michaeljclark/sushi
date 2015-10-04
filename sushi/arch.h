//
//  arch.h
//

#ifndef arch_h
#define arch_h

struct arch
{
	std::string sysname;
	std::string nodename;
	std::string release;
	std::string machine;

	static arch get();

	void print();
	std::string literal();
};

#endif