//
//  arch.cc
//

#include "sushi.h"

#if defined(__linux__) || \
    defined(__FreeBSD__) || \
    defined(__NetBSD__) || \
    defined(__DragonFly__) || \
    defined(__OpenBSD__) || \
    defined(__sun__) || \
    defined(__APPLE__)
#include <sys/utsname.h>
#define HAVE_UTSNAME_H
#elif defined (_WIN32)
#include <windows.h>
typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
#else
#include <unistd.h>
#endif


arch arch::get()
{
	arch info;

#if defined (HAVE_UTSNAME_H)

	struct utsname u;
	if (uname(&u) < 0) {
		log_fatal_exit("uname: %s", strerror(errno));
	}
	info.sysname = u.sysname;
	info.nodename = u.nodename;
	info.release = u.release;
	info.machine = u.machine;

#elif defined (_WIN32)

	OSVERSIONINFO version;
	DWORD nodename_size;
	char nodename[MAX_COMPUTERNAME_LENGTH+1];

	nodename_size = sizeof(nodename);
	if (!GetComputerName(nodename, &nodename_size)) {
		log_fatal_exit("GetComputerName: %d", GetLastError());
	}

	memset(&version, 0, sizeof(version));
	version.dwOSVersionInfoSize = sizeof(version);
	if (!GetVersionEx(&version)) {
		log_fatal_exit("GetVersionEx: %d", GetLastError());
	}

	info.sysname = "Windows";
	info.nodename = nodename;
	std::stringstream release;
	release << version.dwMajorVersion << "." << version.dwMinorVersion;
	info.release = release.str();

#if defined (_M_ARM)
	info.machine = "arm";
#elif defined (_M_X64)
	info.machine = "x86_64";
#elif defined (_M_IX86)
    BOOL is64bit = false;
	LPFN_ISWOW64PROCESS fnIsWow64Process;
	fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(GetModuleHandle("kernel32"), "IsWow64Process");
	if (fnIsWow64Process) {
        fnIsWow64Process(GetCurrentProcess(), &is64bit);
    }
	if (is64bit) {
		info.machine = "x86_64";
	} else {
		info.machine = "x86";
	}
#endif

#else

	char nodename[256];
	if (gethostname(nodename, sizeof(nodename)) < 0) {
		log_fatal_exit("gethostname: %s", strerror(errno));
	}
	info.nodename = nodename;
	info.sysname = "unknown";
	info.release = "unknown";

#if defined (__arm__)
	info.machine = "arm";
#elif defined (__i386__)
	info.machine = "x86";
#elif defined (__x86_64__)
	info.machine = "x86_64";
#elif defined (__sparc__)
	info.machine = "sparc";
#elif defined (__ppc__)
	info.machine = "ppc";
#elif defined (__ppc64__)
	info.machine = "ppc64";
#elif defined (__mips__)
	info.machine = "mips";
#else
	info.machine = "unknown";
#endif

#endif

	return info;
}

void arch::print()
{
	std::cout << sysname << " " << release << " " << machine << " (" << nodename << ")" << std::endl;
}

std::string arch::literal()
{
	std::stringstream lit;
	for (char c : sysname) {
		c = tolower(c);
		if (c == '-' || c == ' ') c = '_';
		lit << c;
	}
	lit << "_";
	for (char c : machine) {
		c = tolower(c);
		lit << c;
	}
	return lit.str();
}
