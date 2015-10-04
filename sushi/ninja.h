//
//  ninja.h
//

#ifndef ninja_h
#define ninja_h

struct Ninja;
struct NinjaVar;
struct NinjaRule;
struct NinjaBuild;
typedef std::shared_ptr<Ninja> NinjaPtr;
typedef std::shared_ptr<NinjaVar> NinjaVarPtr;
typedef std::shared_ptr<NinjaRule> NinjaRulePtr;
typedef std::shared_ptr<NinjaBuild> NinjaBuildPtr;

struct NinjaVar
{
	std::string name;
	std::string value;

	NinjaVar(std::string name, std::string value);
};

struct NinjaRule
{
	std::string name;
	std::map<std::string,std::string> properties;

	NinjaRule(std::string name, std::string command, std::string description);
};

struct NinjaBuild
{
	std::string output;
	std::string rule;
	std::string input;
	std::map<std::string,std::string> properties;

	NinjaBuild(std::string output, std::string rule, std::string input);
};

struct Ninja
{
	std::vector<NinjaVarPtr> ninjaVarList;
	std::vector<NinjaRulePtr> ninjaRuleList;
	std::vector<NinjaBuildPtr> ninjaBuildList;

	static NinjaPtr createBuild(project_root_ptr root);

	void createEmptyBuild(project_root_ptr root, std::map<std::string,std::string> vars);
	void createTarget(project_root_ptr root, std::map<std::string,std::string> vars,
		std::string target_name, std::string target_type,
		std::vector<std::string> depends,
		std::vector<std::string> defines,
		std::vector<std::string> lib_dirs,
		std::vector<std::string> lib_files,
		std::vector<std::string> source);

	void write(project_root_ptr root);
	void write(std::string build_file);
};

#endif