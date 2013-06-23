// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

// This file is meant to be included from a wrapper which should start with:
// #include <config.h>

#include <csignal>  /* signal handlers */
#include <cstdio>
#include <cstdlib>
#ifdef ENABLE_NLS
#include <clocale>
#endif

#include <string>

#include "eixTk/i18n.h"
#include "eixTk/likely.h"
#include "eixTk/null.h"
#include "eixTk/stringutils.h"
#include "main/main.h"

// You must define by a wrapper file - one or several of
//   EIX_BINARY
//   DIFF_BINARY
//   UPDATE_BINARY
//   VERSIONSORT_BINARY
//   MASKED_BINARY
//   DROP_PERMISSIONS_BINARY
// to build the corresponding functionality into the generated binary.
// If several are selected, main() will select depending on the call name.

#undef USE_BINARY
#undef BINARY_COLLECTION

#ifdef EIX_BINARY
#define USE_BINARY run_eix
#endif

#ifdef UPDATE_BINARY
#ifdef USE_BINARY
#define BINARY_COLLECTION 1
#else
#define USE_BINARY run_eix_update
#endif
#endif

#ifdef DIFF_BINARY
#ifdef USE_BINARY
#undef BINARY_COLLECTION
#define BINARY_COLLECTION 1
#else
#define USE_BINARY run_eix_diff
#endif
#endif

#ifdef OVERLAY_BINARY
#ifdef USE_BINARY
#undef BINARY_COLLECTION
#define BINARY_COLLECTION 1
#else
#define USE_BINARY run_eix_overlay
#endif
#endif

#ifdef VERSIONSORT_BINARY
#ifdef USE_BINARY
#undef BINARY_COLLECTION
#define BINARY_COLLECTION 1
#else
#define USE_BINARY run_versionsort
#endif
#endif

#ifdef MASKED_BINARY
#ifdef USE_BINARY
#undef BINARY_COLLECTION
#define BINARY_COLLECTION 1
#else
#define USE_BINARY run_masked_packages
#endif
#endif

#ifdef DROP_PERMISSIONS_BINARY
#ifdef USE_BINARY
#undef BINARY_COLLECTION
#define BINARY_COLLECTION 1
#else
#define USE_BINARY run_eix_drop_permissions
#endif
#endif

#ifdef BINARY_COLLECTION
#undef USE_BINARY
#define USE_BINARY run_program
#endif

#ifndef USE_BINARY
#error "You must #define one of the *BINARY switches, see comments in main.cc"
#endif

using std::string;

/** The name under which we have been called. */
const char *program_name;

static void sig_handler(int sig) ATTRIBUTE_SIGNAL ATTRIBUTE_NORETURN;
static void sanitize_filename(string *s) ATTRIBUTE_NONNULL_;

/** On segfault: show some instructions to help us find the bug. */
static void
sig_handler(int sig)
{
	if(sig == SIGSEGV)
		fprintf(stderr, _(
				"Received SIGSEGV - you probably found a bug in eix.\n"
				"Please proceed with the following few instructions and help us find the bug:\n"
				" * install gdb (sys-dev/gdb)\n"
				" * reemerge eix with FEATURES=\"nostrip\" USE=\"debug\"\n"
				"   or with FEATURES=\"nostrip\" CXXFLAGS=\"-g -ggdb3\" LDFLAGS=\"\"\n"
				" * enter gdb with \"gdb --args %s your_arguments_for_%s\"\n"
				" * type \"run\" and wait for the segfault to happen\n"
				" * type \"bt\" to get a backtrace (this helps us a lot)\n"
				" * post a bugreport and be sure to include the output from gdb ..\n"
				"\n"
				"Sorry for the inconvenience and thanks in advance!\n"),
				program_name, program_name);
	exit(EXIT_FAILURE);
}

/** Cut program path (if there is one) to get only the program name. */
static void
sanitize_filename(string *s)
{
	for(;;) {
		string::size_type i(s->find_last_of('/'));
		if(unlikely(i == string::npos)) {
			return;  // There was no path
		}
		if(likely(++i != s->size())) {
			s->erase(0, i);
		}
		// Trailing '/'. Should not happen, but exec can pass anything.
		i = s->find_last_not_of('/');
		if(unlikely(i == string::npos))
			return;  // Name consists only of '/' - better not touch.
		// Otherwise, cut all trailing / and repeat business
		s->erase(i + 1);
	}
}

#ifdef BINARY_COLLECTION
inline static int run_program(int argc, char *argv[]) ATTRIBUTE_NONNULL_;

inline static int
run_program(int argc, char *argv[])
{
	const string program_lower(to_lower(program_name));
#ifdef DIFF_BINARY
	if(unlikely(program_lower.find("diff") != string::npos))
		return run_eix_diff(argc, argv);
#endif
#ifdef UPDATE_BINARY
#if defined(EIX_BINARY) || defined(OVERLAY_BINARY) || defined(VERSIONSORT_BINARY) || defined(MASKED_BINARY) || defined(DROP_PERMISSIONS_BINARY)
	if(unlikely(program_lower.find("update") != string::npos))
#endif
		return run_eix_update(argc, argv);
#endif
#ifdef DROP_PERMISSIONS_BINARY
#if defined(EIX_BINARY) || defined(OVERLAY_BINARY) || defined(VERSIONSORT_BINARY) || defined(MASKED_BINARY)
	if(likely((program_lower.find("drop") != string::npos) ||
		(program_lower.find("perm") != string::npos)))
#endif
		return run_eix_drop_permissions(argc, argv);
#endif
#ifdef MASKED_BINARY
#if defined(EIX_BINARY) || defined(OVERLAY_BINARY) || defined(VERSIONSORT_BINARY)
	if(likely(program_lower.find("mask") != string::npos))
#endif
		return run_masked_packages(argc, argv);
#endif
#ifdef OVERLAY_BINARY
#if defined(EIX_BINARY) || defined(VERSIONSORT_BINARY)
	if(likely(program_lower.find("overlay") != string::npos))
#endif
		return run_eix_overlay(argc, argv);
#endif
#ifdef VERSIONSORT_BINARY
#if defined(EIX_BINARY)
	if(likely(program_lower.find("vers") != string::npos))
#endif
		return run_versionsort(argc, argv);
#endif
#ifdef EIX_BINARY
		return run_eix(argc, argv);
#endif
}
#endif

int
main(int argc, char** argv)
{
#ifdef ENABLE_NLS
	/* Initialize GNU gettext support */
	setlocale (LC_ALL, "");
	bindtextdomain (PACKAGE, LOCALEDIR);
	textdomain (PACKAGE);
#endif

	/* Install signal handler for segfaults */
#ifdef HAVE_SIGACTION
	static struct sigaction my_handler;
	my_handler.sa_handler = sig_handler;
	my_handler.sa_flags = 0;
	sigemptyset(&(my_handler.sa_mask));
	sigaction(SIGSEGV, &my_handler, NULLPTR);
#else
	signal(SIGSEGV, sig_handler);
#endif

	string my_program_name(argv[0]);
	sanitize_filename(&my_program_name);
	program_name = my_program_name.c_str();
	return USE_BINARY(argc, argv);
}

