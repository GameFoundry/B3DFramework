//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2017 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#ifdef _WIN32
	#define _CRT_SECURE_NO_WARNINGS 1
	#include <direct.h>
	#define getcwd _getcwd
#else
	#include <unistd.h>
#endif

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/mono-config.h>
#include <cstring>

const char* LIB_DIR = "lib/";
const char* ETC_DIR = "etc/";
const char* ASSEMBLIES_DIR = "lib/mono/4.5/";

// Starts up the Mono runtime, runs the managed assembly provided as the first parameter using the
// runtime and passes the rest of the arguments to it. When done shuts down the Mono runtime.
int main(int argc, char* argv[])
{
	// No assembly to run, or Mono directory not provided
	if(argc < 3)
		return 0;

	char* monoDir = argv[2];

	char* libDir = new char[strlen(monoDir) + strlen(LIB_DIR) + 1];
	strcpy(libDir, monoDir);
	strcat(libDir, LIB_DIR);

	char* etcDir = new char[strlen(monoDir) + strlen(ETC_DIR) + 1];
	strcpy(etcDir, monoDir);
	strcat(etcDir, ETC_DIR);

	char* assembliesDir = new char[strlen(monoDir) + strlen(ASSEMBLIES_DIR) + 1];
	strcpy(assembliesDir, monoDir);
	strcat(assembliesDir, ASSEMBLIES_DIR);

	mono_set_dirs(libDir, etcDir);
	mono_set_assemblies_path(assembliesDir);

	delete[] assembliesDir;
	delete[] etcDir;
	delete[] libDir;

	mono_config_parse(nullptr);

	MonoDomain* domain = mono_jit_init(argv[1]);
	if(domain == nullptr)
		return 1;

	MonoAssembly* assembly = mono_domain_assembly_open(domain, argv[1]);
	if(assembly == nullptr)
		return 1;

	int returnVal = mono_jit_exec(domain, assembly, argc - 3, argv + 3);

	mono_jit_cleanup(domain);

	return returnVal;
}
