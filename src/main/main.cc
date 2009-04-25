// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>
#include <main/main.h>
#include <eixTk/exceptions.h>

#include <csignal>  /* signal handlers */
#include <cstdio>

// You must define - in config.h, by -D... or by a wrapper file -
// one or several of
//   EIX_BINARY
//   DIFF_EIX_BINARY
//   UPDATE_BINARY
//   VERSIONSORT_BINARY
// to build the corresponding functionality into the generated binary.
// If several are selected, main() will select depending on the call name.

#undef USE_BINARY
#undef BINARY_COLLECTION

#if defined(EIX_BINARY)
int run_eix(int argc, char *argv[]);
#define USE_BINARY run_eix
#endif

#if defined(UPDATE_EIX_BINARY)
int run_update_eix(int argc, char *argv[]);
#if defined(USE_BINARY)
#define BINARY_COLLECTION 1
#else
#define USE_BINARY run_update_eix
#endif
#endif

#if defined(DIFF_EIX_BINARY)
int run_diff_eix(int argc, char *argv[]);
#if defined(USE_BINARY)
#undef BINARY_COLLECTION
#define BINARY_COLLECTION 1
#else
#define USE_BINARY run_diff_eix
#endif
#endif

#if defined(VERSIONSORT_BINARY)
int run_versionsort(int argc, char *argv[]);
#if defined(USE_BINARY)
#undef BINARY_COLLECTION
#define BINARY_COLLECTION 1
#else
#define USE_BINARY run_versionsort
#endif
#endif

#if defined(BINARY_COLLECTION)
static int run_program(int argc, char *argv[]);
#undef USE_BINARY
#define USE_BINARY run_program
#endif

#if !defined(USE_BINARY)
#error "You must #define one of the *BINARY switches, see comments in main.cc"
#endif

using namespace std;

/** The name under which we have been called. */
string program_name;

/** On segfault: show some instructions to help us find the bug. */
static void
sig_handler(int sig)
{
	if(sig == SIGSEGV)
		fprintf(stderr,
				"Received SIGSEGV - you probably found a bug in eix.\n"
				"Please proceed with the following few instructions and help us find the bug:\n"
				" * install gdb (sys-dev/gdb)\n"
				" * compile eix with FEATURES=\"nostrip\" CXXFLAGS=\"-g -ggdb3\"\n"
				" * enter gdb with \"gdb --args %s your_arguments_for_%s\"\n"
				" * type \"run\" and wait for the segfault to happen\n"
				" * type \"bt\" to get a backtrace (this helps us a lot)\n"
				" * post a bugreport and be sure to include the output from gdb ..\n"
				"\n"
				"Sorry for the inconvenience and thanks in advance!\n",
				program_name.c_str(), program_name.c_str());
	exit(1);
}

/** Cut program path (if there is one) to get only the program name. */
static void
sanitize_filename(string &s)
{
	for(;;) {
		string::size_type i = s.find_last_of('/');
		if( i == string::npos )
			return;// There was no path
		if(++i != s.size()) {
			s.erase(0, i);
			// Since, by definition, this is called only once, we
			// need no memory management: Just one static string.
			static string keep(s);
			s = keep.c_str();
		}
		// Trailing '/'. Should not happen, but exec can pass anything.
		i = s.find_last_not_of('/');
		if(i == string::npos)
			return;// Name consists only of '/' - better not touch.
		// Otherwise, cut all trailing / and repeat business
		s.erase(i + 1);
	}
}

#if defined(BINARY_COLLECTION)
static int
run_program(int argc, char *argv[])
{
#if defined(DIFF_EIX_BINARY)
	if((program_name.find("diff") != string::npos) ||
		(program_name.find("DIFF") != string::npos))
		return run_diff_eix(argc, argv);
#endif
#if defined(UPDATE_EIX_BINARY)
#if defined(EIX_BINARY) || defined(VERSIONSORT_BINARY)
	if((program_name.find("update") != string::npos) ||
		(program_name.find("UPDATE") != string::npos))
#endif
		return run_update_eix(argc, argv);
#endif
#if defined(VERSIONSORT_BINARY)
#if defined(EIX_BINARY)
	if((program_name.find("vers") != string::npos) ||
		(program_name.find("VERS") != string::npos))
#endif
		return run_versionsort(argc, argv);
#endif
#if defined(EIX_BINARY)
		return run_eix(argc, argv);
#endif
}
#endif

int
main(int argc, char** argv)
{
	/* Install signal handler for segfaults */
	signal(SIGSEGV, sig_handler);

	int ret = 0;
	try {
		program_name = argv[0];
		sanitize_filename(program_name);
		ret = USE_BINARY(argc, argv);
	}
	catch(const ExBasic &e) {
		cout << e << endl;
		return 1;
	}
	return ret;
}

