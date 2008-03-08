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

#include <string>

#include <csignal>  /* signal handlers */


// This is a bit tricky, since it should work universal:
// You must define - either in config.h or by -D... - at least one of
//   ALL_BINARY
//   OUTPUT_BINARY (diff-eix + eix)
//   EIX_BINARY
//   DIFF_EIX_BINARY
//   UPDATE_BINARY
// to build the corresponding functionality into the generated binary.
// If several functionalities are required, BINARY_COLECTION will be defined
// by this header, and main() will select the correct one by the call name.

#if defined(ALL_BINARY)
#if !defined(UPDATE_EIX_BINARY)
#define UPDATE_EIX_BINARY 1
#endif
#if !defined(OUTPUT_BINARY)
#define OUTPUT_BINARY 1
#endif
#endif

#if defined(OUTPUT_BINARY)
#if !defined(EIX_BINARY)
#define EIX_BINARY 1
#endif
#if !defined(DIFF_EIX_BINARY)
#define DIFF_EIX_BINARY 1
#endif
#endif

#undef HAVE_A_BINARY
#undef BINARY_COLLECTION

#if defined(EIX_BINARY)
#if defined(HAVE_A_BINARY)
#define BINARY_COLLECTION 1
#else
#define HAVE_A_BINARY 1
#endif
int run_eix(int argc, char *argv[]);
#endif

#if defined(UPDATE_EIX_BINARY)
#if defined(HAVE_A_BINARY)
#define BINARY_COLLECTION 1
#else
#define HAVE_A_BINARY 1
#endif
int run_update_eix(int argc, char *argv[]);
#endif

#if defined(DIFF_EIX_BINARY)
#if defined(HAVE_A_BINARY)
#define BINARY_COLLECTION 1
#else
#define HAVE_A_BINARY 1
#endif
int run_diff_eix(int argc, char *argv[]);
#endif

#if !defined(HAVE_A_BINARY)
#error "You must #define one of the *BINARY switches, see comments in main.cc"
#endif

using namespace std;

/** The name under which we have been called. */
const char *program_name;

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
				program_name, program_name);
	exit(1);
}

#if defined(BINARY_COLLECTION)
int
run_program(int argc, char *argv[])
{
	string s(program_name);
	{
		string::size_type i = s.find_last_of('/');
		if(i != string::npos)
			s.erase(0, i + 1);
	}
#if defined(DIFF_EIX_BINARY)
	if((s.find("diff") != string::npos) || (s.find("DIFF") != string::npos))
		return run_diff_eix(argc, argv);
#endif
#if defined(UPDATE_EIX_BINARY)
#if defined(EIX_BINARY)
	if((s.find("update") != string::npos) || (s.find("UPDATE") != string::npos))
#endif
		return run_update_eix(argc, argv);
#endif
#if defined(EIX_BINARY)
		return run_eix(argc, argv);
#endif
}
#endif

int
main(int argc, char** argv)
{
	program_name = argv[0];

	/* Install signal handler for segfaults */
	signal(SIGSEGV, sig_handler);

	int ret = 0;
	try {
#if defined(BINARY_COLLECTION)
		ret = run_program(argc, argv);
#else
#if defined(EIX_BINARY)
		ret = run_eix(argc, argv);
#endif
#if defined(DIFF_EIX_BINARY)
		ret = run_diff_eix(argc, argv);
#endif
#if defined(UPDATE_EIX_BINARY)
		ret = run_update_eix(argc, argv);
#endif
#endif
	}
	catch(const ExBasic &e) {
		cout << e << endl;
		return 1;
	}
	return ret;
}
