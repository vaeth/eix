project('eix', 'cpp',
	version : '0.35.2',
	license : 'GPLv2',
	default_options : [
		'prefix=/usr',
		'sysconfdir=/etc',
	],
	meson_version : '>= 0.41', # support configure_file(capture : true)
)

jumbo_build = get_option('jumbo-build')
new_dialect = get_option('new-dialect')
dead_code = get_option('dead-code')
debugging = get_option('debugging')
debug_format = get_option('debug-format')
paranoic_asserts = get_option('paranoic-asserts')
security = get_option('security')
strong_security = get_option('strong-security')
nopie_security = get_option('nopie-security')
normal_optimization = get_option('normal-optimization')
strong_optimization = get_option('strong-optimization')
warnings = get_option('warnings')
strong_warnings = get_option('strong-warnings')

dead_code_msg = ''
normal_optimization_msg = ''
strong_optimization_msg = ''
paranoic_asserts_msg = ''
warnings_msg = ''
security_msg = ''
strong_security_msg = ''

if debugging
	dead_code = false
	dead_code_msg = ' (disabled by debugging)'
endif
if debugging and not paranoic_asserts
	paranoic_asserts = true
	paranoic_asserts_msg = ' (forced by debugging)'
endif
if debugging and strong_optimization
	strong_optimization = false
	strong_optimization_msg = ' (disabled by debugging)'
endif
if debugging and normal_optimization
	normal_optimization = false
	normal_optimization_msg = ' (disabled by debugging)'
endif
if strong_optimization and not normal_optimization
	normal_optimization = true
	normal_optimization_msg = ' (forced by strong-optimization)'
endif
if paranoic_asserts and strong_optimization
	strong_optimization = false
	strong_optimization_msg = ' (disabled by paranoic-asserts)'
endif
if strong_warnings and not warnings
	warnings = true
	warnings_msg = ' (forced by strong-warnings)'
endif
if nopie_security and not strong_security
	strong_security = true
	strong_security_msg = ' (forced by nopie-security)'
endif
if strong_security and not security
	security = true
	security_msg = ' (forced by strong-security)'
endif

result = [ 'Passed options:' ]
result += [ 'jumbo-build=' + jumbo_build.to_string() ]
result += [ 'debugging=' + debugging.to_string() ]
result += [ 'debug-format=' + debug_format.to_string() ]
result += [ 'paranoic-asserts=' + paranoic_asserts.to_string() + paranoic_asserts_msg ]
result += [ 'security=' + security.to_string() + security_msg ]
result += [ 'strong_security=' + strong_security.to_string() + strong_security_msg ]
result += [ 'nopie_security=' + nopie_security.to_string() ]
result += [ 'normal-optimization=' + normal_optimization.to_string() + normal_optimization_msg ]
result += [ 'strong-optimization=' + strong_optimization.to_string() + strong_optimization_msg ]
result += [ 'warnings=' + warnings.to_string() + warnings_msg ]
result += [ 'strong-warnings=' + strong_warnings.to_string() ]

cxx = meson.get_compiler('cpp')
cxx_id = cxx.get_id()
cross = meson.is_cross_build()
sh = find_program('sh')
sed = find_program('sed')
echo = find_program('echo')
incdir = include_directories('src')

builddir = meson.current_build_dir()
sourcedir = meson.current_source_dir()
package = meson.project_name()
version = meson.project_version()
prefix = get_option('prefix')
bindir = join_paths(prefix, get_option('bindir'))
datadir = join_paths(prefix, get_option('datadir'))
sysconfdir = get_option('sysconfdir')
localedir = join_paths(prefix, get_option('localedir'))
mandir = join_paths(prefix, get_option('mandir'))

docdir = get_option('docdir')
if docdir != ''
	docdir = join_paths(prefix, docdir)
else
	docdir = join_paths(prefix, datadir, 'doc', package)
endif

htmldir = get_option('htmldir')
if htmldir != ''
	htmldir = join_paths(prefix, htmldir)
else
	htmldir = join_paths(docdir, 'html')
endif

ldflags_add = []
cxxflags_add = []

flto = false
r = run_command(sh, '-c', '''case " ${CXXFLAGS} ${LDFLAGS} " in
'-flto '*|'flto='*) exit 1;;
esac
''')
if r.returncode() != 0
	flto = true
endif

# Explanations:
# The mere inclusion of <map> causes errors on some clang + -std= combinations
# Similarly, the is_function is sometimes not resolved correctly.
# Do further some stress test with vectors and strings
stresstest = '''#include <map>
#include <unistd.h>
#include <cstring>
#include <string>
#include <vector>
int my_func();
int my_func() {
	std::vector<const char*> a(1, "a"); int my_a[1000];
	for (std::vector<const char*>::size_type j = 999; j < 999 + a.size(); ++j)
		my_a[j] = 0;
	return (strchr(a[0], *a[0]) == *(a.begin())) ? my_a[999] : 1;
}
#ifdef __cplusplus
#if __cplusplus >= 201103L
#include <type_traits>
void func_noexcept() noexcept;
void func_noexcept() noexcept {}
#define C11TESTCALL 1
#endif
#endif
int main() {
#ifdef C11TESTCALL
	if(!std::is_function<decltype(func_noexcept)>::value) {
		return 1;
	}
#endif
	return my_func();
}
'''

flags_fatal = []
foreach o : [
	'-Werror',
	'-Werror=unknown-warning-option',
	'-Wunknown-warning-option',
	'-Wl,--fatal-warnings',
	'-Wattributes',
]
	t = flags_fatal
	t += [ o ]
	if cxx.links(stresstest, args : t)
		flags_fatal += [ o ]
	endif
endforeach

if strong_optimization
	flto_test = []
	if cxx_id == 'clang'
		flto_test += [
			'-emit-llvm',
		]
	else
		flto_test += [
			'-use-linker-plugin',
		]
	endif
	flto_test += [
		'-flto',
		'-flto-partition=none',
		'-flto-odr-type-merging',
		'-fwhole-program',
		'-fno-fat-lto-objects',
	]
	flto_flags = []
	foreach o : flto_test
		t = flags_fatal
		t += flto_flags
		t += [ o ]
		if not flto or o != '-flto'
			if cross
				s = cxx.links(stresstest, args : t)
			else
				s = cxx.run(stresstest, args : t)
				s = (s.compiled() and s.returncode() == 0)
			endif
			if s
				flto_flags += [ o ]
				if not flto and o == '-flto'
					flto = true
				endif
			endif
		endif
	endforeach
	if flto
		foreach o : flto_flags
			message('CXXLDFLAGS: ' + o)
			cxxflags_add += [ o ]
			add_global_arguments(o, language : 'cpp')
			ldflags_add += [ o ]
			add_global_link_arguments(o, language : 'cpp')
		endforeach
	endif
endif

flags_dialect = []
dialect = 'false'
if new_dialect
	foreach d : [
		'gnu++23',
		'c++23',
		'gnu++2b',
		'c++2b',
		'gnu++20',
		'c++20',
		'gnu++2a',
		'c++2a',
		'gnu++17',
		'c++17',
		'gnu++1z',
		'c++1z',
		'gnu++14',
		'c++14',
		'gnu++1y',
		'c++1y',
		'gnu++11',
		'c++11',
	]
		if dialect == 'false'
			o = '-std=' + d
			t = flags_fatal
			t += [ o ]
			if cxx.links(stresstest, args : t)
				dialect = d
				flags_dialect += [ o ]
				flags_fatal += [ o ]
				cxxflags_add += [ o ]
				add_global_arguments(o, language : 'cpp')
				if flto
					message('CXXLDFLAGS: ' + o)
					ldflags_add += [ o ]
					add_global_link_arguments(o, language : 'cpp')
				else
					message('CXXFLAGS: ' + o)
				endif
			endif
		endif
	endforeach
	result += [ 'new-dialect=' + 'true -> ' + dialect ]
else
	result += [ 'new-dialect=false' ]
endif

ldflags_force = []
ldflags_opt = []
cxxflags_opt = []
ldcxxflags_opt = []

if dead_code
	o = '-Wl,--gc-sections'
	t = flags_fatal
	t += [ o ]
	if cxx.links(stresstest, args : t)
		ldflags_force += [ o ]
		cxxflags_opt += [
			[ '-fdata-sections' ],
			[ '-ffunction-sections' ],
		]
	else
		dead_code = false
		dead_code_msg = ' (not supported)'
	endif
endif
result += [ 'dead-code=' + dead_code.to_string() + dead_code_msg ]

if debugging
	ldcxxflags_opt += [
		[ '-g' ],
		[ '-ggdb3' ],
	]
endif
if normal_optimization
	cxxflags_opt += [
		[ '-O2' ],
		[ '-fomit-frame-pointer' ],
		[ '-fno-common' ],
		[ '-fstrict-aliasing' ],
		[ '-fstrict-enums' ],
		[ '-fmerge-all-constants' ],
		[ '-finline-functions' ],
		[ '-fvisibility=hidden' ],
		[ '-fvisibility-inlines-hidden' ],
		[ '-ftree-pre' ],
		[ '-ftree-partial-pre' ],
		[ '-fnothrow-opt' ],
		[ '-fno-unwind-tables' ],
		[ '-fno-asynchronous-unwind-tables' ],
		[ '-ffast-math' ],
		[ '-fno-math-errno' ],
		[ '-frename-registers' ],
		[ '-funsafe-loop-optimizations' ],
		[ '-fgcse-sm' ],
		[ '-fgcse-las' ],
		[ '-fgcse-after-reload' ],
		[ '-fpredictive-commoning' ],
		[ '-ftree-switch-conversion' ],
		[ '-fno-ident' ],
		[ '-freorder-functions' ],
		[ '-fdevirtualize-speculatively' ],
		[ '-fdevirtualize-at-ltrans' ],
		[ '-fno-semantic-interposition' ],
		[ '-fira-loop-pressure' ],
		[ '-ftree-loop-distribution' ],
		[ '-ftree-loop-distribute-patterns' ],
		[ '-ftree-loop-im' ],
		[ '-fivopts' ],
		[ '-fdelete-null-pointer-checks' ],
	]
	ldflags_opt += [
		[ '-Wl,-O1' ],
		[ '-Wl,--relax' ],
		[ '-Wl,--as-needed' ],
		[ '-Wl,--sort-common' ],
		[ '-Wl,-z,combreloc' ],
		[ '-Wl,--build-id=none' ],
	]
endif
if strong_optimization
	cxxflags_opt += [
		[ '-fweb' ],
		# [ '-fgraphite' ],
		# [ '-fgraphite-identity' ],
		# [ '-floop-interchange' ],
		# [ '-floop-strip-mine' ],
		# [ '-floop-block' ],
		# [ '-floop-next-optimize' ],
		# [ '-floop-unroll-and-jam' ],
		# [ '-fno-rtti' ],
		[ '-fno-enforce-eh-specs' ],
		[ '-ftree-vectorize' ],
		[ '-fvect-cost-model' ],
		[ '-fno-threadsafe-statics' ],
	]
	ldflags_opt += [
		[ '-Wl,-O9' ],
		[ '-Wl,-z,noexecstack' ],
		# [ '-Wl,--hash-style=gnu' ],
		# [ '-Wl,--enable-new-dtags' ],
	]
	if not strong_security
		cxxflags_opt += [
			[ '-DNDEBUG' ],
			[ '-DNO_DEBUG' ],
			[ '-DG_DISABLE_ASSERT' ],
		]
	endif
endif
if strong_warnings
	ldflags_opt += [
		[ '-Wl,--warn-common' ],
	]
endif
if warnings
	if dialect != ''
		ldcxxflags_opt += [
			[ '-Wzero-as-null-pointer-constant' ],
		]
	endif
	cxxflags_opt += [
		[ '-fno-common' ],
		[ '-funsigned-char' ],
		[ '-ftracer' ],
		[ '-fconcepts' ],
		[ '-Wpedantic' ],
		[ '-pedantic' ],
		[ '-Wall' ],
		[ '-Wextra' ],
		[ '-Wformat=2' ],
		[ '-Wunused' ],
		[ '-Wunused-variable' ],
		[ '-Wunused-parameter' ],
		[ '-Winit-self' ],
		[ '-Wmissing-include-dirs' ],
		[ '-Wswitch-default' ],
		[ '-Wstrict-aliasing=1' ],
		[ '-Wstrict-overflow' ],
		[ '-Wfloat-equal' ],
		[ '-Wundef' ],
		[ '-Wshadow' ],
		[ '-Wpointer-arith' ],
		[ '-Wcast-qual' ],
		[
			'-Wcast-align=strict',
			'-Wcast-align',
		],
		[ '-Wwrite-strings' ],
		[ '-Wsign-compare' ],
		[ '-Wmissing-field-initializers' ],
		[ '-Wnormalized=nfkc' ],
		[ '-Wpacked' ],
		[ '-Wredundant-decls' ],
		[ '-Winvalid-pch' ],
		[ '-Wvolatile-register-var' ],
		[ '-Wdisabled-optimization' ],
		[ '-Wlogical-op' ],
		[ '-Wvla' ],
		[ '-Wabi' ],
		[ '-Wctor-dtor-privacy' ],
		[ '-Wstrict-null-sentinel' ],
		[ '-Wstring-plus-int' ],
		[ '-Woverloaded-virtual' ],
		[ '-Wsign-promo' ],
		[ '-Wnoexcept' ],
		[ '-Wmissing-declarations' ],
		[ '-Wmissing-format-attribute' ],
		[ '-Wunused-but-set-variable' ],
		[ '-Wunused-but-set-parameter' ],
		[ '-Wsuggest-attribute=pure' ],
		[ '-Wsuggest-attribute=const' ],
		[ '-Wsuggest-attribute=noreturn' ],
		[ '-Wsuggest-attribute=malloc' ],
		[ '-Wsuggest-attribute=cold' ],
		[ '-Wsuggest-final-type' ],
		[ '-Wsuggest-final-methods' ],
		[ '-Wsuggest-override' ],
		[ '-Wdelete-non-virtual-dtor' ],
		[ '-Wint-to-pointer-cast' ],
		[ '-Wcomment' ],
		[ '-WNSObject-attribute' ],
		[ '-fdiagnostic-color=always' ],
		[ '-Wswitch-bool' ],
		[ '-Wlogical-not-parantheses' ],
		[ '-Wsizeof-array-argument' ],
		[ '-Walloc-zero' ],
		[ '-Wbool-compare' ],
		[ '-Wbool-operation' ],
		[ '-Wodr-type-mismatch' ],
		[ '-Wshift-negative-value' ],
		[ '-Wshift-overflow' ],
		[ '-Wshift-overflow=2' ],
		[ '-Wnull-dereference' ],
		[ '-Wduplicated-branches' ],
		[ '-Wduplicated-cond' ],
		[ '-Wstringop-truncation' ],
		[ '-Wctad-maybe-unsupported' ],
		# [ '-Weverything' ],
		# [ '-Wno-unknown-warning-option' ],
		# [ '-fmem-report' ],
		# [ '-Wtraditional' ],
		# [ '-Wc++-compat' ],
		# [ '-Wstrict-prototypes' ],
		# [ '-Wold-style-definition' ],
		# [ '-Wmissing-prototypes' ],
		# [ '-Wnested-externs' ],
		# [ '-Wpointer-sign' ],
		# [ '-Wtautological-compare' ],
		# [ '-Wmisleading-indentation' ],
	]
	ldflags_opt += [
		[ '-Wl,-z,defs' ],
		[ '-Wl,--no-undefined' ],
		[ '-Wl,--no-allow-shlib-undefined' ],
		[ '-Wl,--no-undefined-version' ],
	]
endif
if strong_warnings
	cxxflags_opt += [
		# [ '-Wconversion' ],
		# [ '-Wswitch-enum' ],
		# [ '-Wunsafe-loop-optimizations' ],
		# [ '-Wpadded' ],
		# [ '-Wunreachable-code' ],
		# [ '-Waggregate-return' ],
		# [ '-Winline' ],
		# [ '-Weffc++' ],
		[ '-Wsign-conversion' ],
		[ '-Wold-style-cast' ],
		[ '-Wmissing-noreturn' ],
	]
endif

if security
	if strong_security
		ldcxxflags_opt += [
			[
				'-fstack-protector-strong',
				'-fstack-protector-all',
				'-fstack-protector',
			]
		]
	else
		ldcxxflags_opt += [
			[
				'-fstack-protector-all',
				'-fstack-protector-strong',
				'-fstack-protector',
			]
		]
	endif
	ldcxxflags_opt += [
		[
			'-fstack-clash-protection',
			'-fstack-check=specific',
			'-fstack-check',
		]
	]
	cxxflags_opt += [
		# [ '-fno-plt' ], # breaks with clang; no proper check known
		[ '-mretpoline' ],
		[ '-mcet' ],
		[ '-fcf-protection=full' ],
		# [ '-mindirect-branch=thunk' ], # breaks with cygwin; no proper check known
		# [ '-mfunction-return=thunk' ],
		[ '-D_FORTIFY_SOURCE=2' ],
	]
	ldflags_opt += [
		[ '-Wl,-z,retpolineplt' ],
		[ '-Wl,-z,now' ],
		[ '-Wl,-z,relro' ],
		[ '-Wl,-z,noexecstack' ],
	]
	if not nopie_security
		ldcxxflags_opt += [
			[ '-fPIE' ],
			[ '-pie' ],
		]
	endif
endif
if strong_security
	ldcxxflags_opt += [
		[ '-faddress-sanitizer' ],
		[ '-fsanitize=undefined' ],
		[ '-fsanitize=vla-bound' ],
		[ '-fsanitize=unreachable' ],
		[ '-fsanitize=null' ],
		[ '-fsanitize=return' ],
		[ '-fsanitize=float-divide-by-zero' ],
		[ '-fsanitize=bounds' ],
		[ '-fsanitize=bounds-strict' ],
		[ '-fsanitize=alignment' ],
		[ '-fsanitize=pointer-compare' ],
		[ '-fsanitize=pointer-subtract' ],
		# [ '-fsanitize=address' ],
		# [ '-fsanitize=leak' ],
		# [ '-fsanitize=shift' ],
		# [ '-fsanitize=signed-integer-overflow' ],
		# [ '-fsanitize=integer-divide-by-zero' ],
		# [ '-fsanitize=float-cast-overflow' ],
		# [ '-fsanitize=object-size' ],
		# [ '-fsanitize=threads' ],
		# [ '-fsanitize=vptr' ],
		# [ '-mmpx' ],
		# [ '-fcheck-pointer-bounds' ],
		# [ '-Wchkp' ],
	]
endif

foreach o : ldflags_force
	ldflags_add += [ o ]
	add_global_link_arguments(o, language : 'cpp')
endforeach

foreach c : ldcxxflags_opt
	check_rest = true
	foreach o : c
		t = flags_fatal
		t += ldflags_add
		t += [ o ]
		if cxx.links(stresstest, args : t)
			message('LDFLAGS: ' + o)
			ldflags_add += [ o ]
			add_global_link_arguments(o, language : 'cpp')
			check_rest = false
		endif
	endforeach
	if check_rest
		t = 'NO:'
		foreach o : c
			t += ' ' + o
		endforeach
		message(t)
	endif
endforeach

foreach c : ldcxxflags_opt
	check_rest = true
	foreach o : c
		if check_rest
			t = flags_fatal
			t += cxxflags_add
			t += ldflags_add
			t += [ o ]
			if cxx.links(stresstest, args : t)
				message('CXXLDFLAGS: ' + o)
				cxxflags_add += [ o ]
				add_global_arguments(o, language : 'cpp')
				ldflags_add += [ o ]
				add_global_link_arguments(o, language : 'cpp')
				check_rest = false
			endif
		endif
	endforeach
	if check_rest
		t = 'NO:'
		foreach o : c
			t += ' ' + o
		endforeach
		message(t)
	endif
endforeach

foreach c : cxxflags_opt
	check_rest = true
	foreach o : c
		if check_rest
			t = flags_fatal
			if flto
				t += ldflags_add
			endif
			t += cxxflags_add
			t += [ o ]
			if cxx.links(stresstest, args : t)
				cxxflags_add += [ o ]
				add_global_arguments(o, language : 'cpp')
				if flto
					message('CXXLDFLAGS: ' + o)
					ldflags_add += [ o ]
					add_global_link_arguments(o, language : 'cpp')
				else
					message('CXXFLAGS: ' + o)
				endif
				check_rest = false
			endif
		endif
	endforeach
	if check_rest
		t = 'NO:'
		foreach o : c
			t += ' ' + o
		endforeach
		message(t)
	endif
endforeach

vers = configuration_data()
vers.set_quoted('PACKAGE_VERSION', version)
subst = configuration_data()
subst.set('PACKAGE_STRING', package + ' ' + version)
subst.set('PACKAGE_BUGREPORT', 'https://github.com/vaeth/eix/issues/')
subst.set('PACKAGE_URL', 'https://github.com/vaeth/eix/')

subst.set('BINDIR', bindir)

subst.set('SYSCONFDIR', sysconfdir)
conf = configuration_data()
conf.set_quoted('SYSCONFDIR', sysconfdir,
	description : 'Path to the configuration directory, usually /etc')

subst.set('LOCALEDIR', localedir)
conf.set_quoted('LOCALEDIR', localedir,
	description : 'Path to the locale directory, usually /usr/share/locale')

conf.set('JUMBO_BUILD', jumbo_build,
	description : 'Define if jumbo build should be used')
conf.set('EIX_PARANOIC_ASSERTS', paranoic_asserts,
	description : 'Define if paranoic asserts should be used')
conf.set('EIX_STATIC_ASSERTS', paranoic_asserts,
	description : 'Define if asserts for static initializers should be used')
conf.set('EIX_DEBUG_FORMAT', debug_format,
	description : 'Define if string format runtime tests should be used')

dev_null = get_option('dev-null')
result += [ 'dev-null=' + dev_null ]
subst.set('DEV_NULL', dev_null)
conf.set_quoted('DEV_NULL', dev_null,
	description : 'null device of the system, usually /dev/null')

nls = get_option('nls')
result += [ 'nls=' + nls.to_string() ]
manpages = [ 'en' ]
if nls
	subdir('po')
endif
subst.set('USE_NLS', nls ? 'yes' : 'no')
subst.set('TEXTDOMAIN', package)
subst.set('TEXTDOMAINDIR', localedir)

check_headers = [
	['HAVE_CLIMITS', 'climits'],
	['HAVE_CSTDINT', 'cstdint'],
	['HAVE_GRP_H', 'grp.h'],
	['HAVE_INTERIX_SECURITY_H', 'interix/security.h'],
	['HAVE_LIMITS_H', 'limits.h'],
	['HAVE_STDINT_H', 'stdint.h'],
	['HAVE_STROPTS_H', 'stropts.h'],
	['HAVE_SYS_FILE_H', 'sys/file.h'],
	['HAVE_SYS_IOCTL_H', 'sys/ioctl.h'],
	['HAVE_SYS_PARAM_H', 'sys/param.h'],
	['HAVE_SYS_PTEM_H', 'sys/ptem.h'],
	['HAVE_SYS_PTY_H', 'sys/pty.h'],
	['HAVE_SYS_STREAM_H', 'sys/stream.h'],
	['HAVE_SYS_TTY_H', 'sys/tty.h'],
	['HAVE_TERMIOS_H', 'termios.h'],
	['HAVE_TR1_CSTDINT', 'tr1/cstdint'],
]
foreach p : check_headers
	h = p.get(1)
	conf.set(p.get(0), cxx.has_header(h),
		description : 'Define if ' + h + ' can be included')
endforeach

cdefines = ''

minix = cxx.has_header('minix/config.h')
conf.set('_MINIX', minix ? '1' : false,
	description : 'Define to 1 if on MINIX')
conf.set('_POSIX_SOURCE', minix ? '1' : false,
	description : 'Define to 1 if needed, e.g. for stat')
conf.set('_POSIX_1_SOURCE', minix ? '2' : false,
	description : 'Define to 2 if needed for POSIX.1 features')
if minix
	cdefines += '''
#define _MINIX 1
#define _POSIX_SOURCE 1
#define _POSIX_1_SOURCE 2
'''
	message('Support Minix')
else
	message('Minix: NO')
endif

cheaders = cdefines + '''
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
'''
if conf.get('HAVE_SYS_FILE_H')
	cheaders += '#include <sys/file.h>\n'
endif
if conf.get('HAVE_INTERIX_SECURITY_H')
	cheaders += '#include <interix/security.h>\n'
endif
if conf.get('HAVE_GRP_H')
	cheaders += '#include <grp.h>\n'
endif
cheaders += '''
#include <cstdlib>
#include <cstring>
'''

cextensions = '''
#define __EXTENSIONS__ 1
#define _ALL_SOURCE 1
#define _GNU_SOURCE 1
#define _POSIX_PTHREAD_SEMANTICS 1
#define _TANDEM_SOURCE 1
'''
extensions = cxx.links(cextensions + cheaders + '''
int main() { return 0; }
''', args : flags_fatal)
if extensions
	cheaders = cextensions + cheaders
	message('Support Extensions')
else
	message('Extensions: NO')
endif
conf.set('__EXTENSIONS__', extensions ? '1' : false,
	description : 'Define to 1 to enable general extensions on Solaris')
conf.set('_ALL_SOURCE', extensions ? '1' : false,
	description : 'Define to 1 to enable GNU extensions')
conf.set('_POSIX_PTHREAD_SEMANTICS', extensions ? '1' : false,
	description : 'Define to 1 to enable threading extensions on Solaris')
conf.set('_TANDEM_SOURCE', extensions ? '1' : false,
	description : 'Define to 1 to enable extensions on HP NonStop')

check_functions = [
	['HAVE_ATOI', 'atoi'],
	['HAVE_CANONICALIZE_FILE_NAME', 'canonicalize_file_name'],
	['HAVE_FILENO', 'fileno'],
	['HAVE_FLOCK', 'flock'],
	['HAVE_FSEEKO', 'fseeko'],
	['HAVE_GETEGID', 'getegid'],
	['HAVE_GETEUID', 'geteuid'],
	['HAVE_GETGID', 'getgid'],
	['HAVE_GETUID', 'getuid'],
	['HAVE_INITGROUPS', 'initgroups'],
	['HAVE_REALPATH', 'realpath'],
	['HAVE_SETEGID', 'setegid'],
	['HAVE_SETENV', 'setenv'],
	['HAVE_SETEUID', 'seteuid'],
	['HAVE_SETGID', 'setgid'],
	['HAVE_SETGROUPS', 'setgroups'],
	['HAVE_SETUID', 'setuid'],
	['HAVE_SETUSER', 'setuser'],
	['HAVE_SIGACTION', 'sigaction'],
	['HAVE_STRTOL', 'strtol'],
	['HAVE_STRTOLL', 'strtoll'],
	['HAVE_STRTOUL', 'strtoul'],
	['HAVE_STRTOULL', 'strtoull'],
	['HAVE_VFORK', 'vfork'],
]
foreach p : check_functions
	f = p.get(1)
	conf.set(p.get(0), cxx.has_function(f, prefix : cheaders),
		description : 'Define if ' + f + '() is available')
endforeach

foreach p : [
	['getegid', 'gid_t seteuid()', ''],
	['geteuid', 'uid_t seteuid()', ''],
	['getgid', 'git_d setgid()', ''],
	['getuid', 'uid_t setuid()', ''],
	['setegid', 'int setegid(gid_t gid)', '0'],
	['seteuid', 'int seteuid(uid_t uid)', '0'],
	['setgid', 'int setgid(gid_t gid)', '0'],
	['setuid', 'int setuid(uid_t uid)', '0'],
]
	program = 'int main() {\n'
	program += 'return !' + p.get(0) + '(' + p.get(2) + ');\n}'
	broken = not cxx.links(cheaders + program, args : flags_dialect)
	conf.set('NEED_' + p.get(0).to_upper() + '_PROTO', broken,
		description : 'Define if ' + p.get(0) + ' needs a prototype')
	if broken
		broken = not cxx.links(cheaders + p.get(1) + ';\n' + program)
		if broken
			message(p.get(0) + '() is broken')
		else
			message(p.get(0) + '() needs a prototype')
		endif
	else
		message(p.get(0) + '() works')
	endif
	conf.set('BROKEN_' + p.get(0).to_upper(), broken,
		description : 'Define if ' + p.get(0) + ' is broken')
endforeach

builtin_expect = cxx.links('''
int main() {
int zero = 0;
int one = 1;
if(!((__builtin_expect(one, 0)) && (__builtin_expect(one, 1)) &&
	(!__builtin_expect(zero, 0)) && (!__builtin_expect(zero, 1))))
	return 1;
}
''', args : flags_dialect)
message('__builtin_expect: ' + builtin_expect.to_string())
conf.set('HAVE___BUILTIN_EXPECT', builtin_expect,
	description : 'Define if __builtin_expect can be used')

pragma_diagnostic = cxx.links('''
#define GCC_DIAG_STR(s) #s
#define GCC_DIAG_JOINSTR(x, y) GCC_DIAG_STR(x ## y)
#define GCC_DIAG_DO_PRAGMA(x) _Pragma (#x)
#define GCC_DIAG_PRAGMA(x) GCC_DIAG_DO_PRAGMA(GCC diagnostic x)
#define GCC_DIAG_OFF(x) GCC_DIAG_PRAGMA(push) \
	GCC_DIAG_PRAGMA(ignored GCC_DIAG_JOINSTR(-W, x))
#define GCC_DIAG_ON(x) GCC_DIAG_PRAGMA(pop)
int main() {
signed char j(0);
unsigned char i;
GCC_DIAG_OFF(sign-conversion)
i = j;
GCC_DIAG_ON(sign-conversion)
GCC_DIAG_OFF(old-style-cast)
return ((int)i);
GCC_DIAG_ON(old-style-cast)
}
''', args : flags_dialect)
message('_Pragma GCC diagnostic: ' + pragma_diagnostic.to_string())
conf.set('FULL_GCC_DIAG_PRAGMA', pragma_diagnostic,
	description : 'Define if _Pragma GCC diagnostic can be used everywhere')

wsuggest_final_methods = cxx.links('''
#define GCC_DIAG_STR(s) #s
#define GCC_DIAG_JOINSTR(x, y) GCC_DIAG_STR(x ## y)
#define GCC_DIAG_DO_PRAGMA(x) _Pragma (#x)
#define GCC_DIAG_PRAGMA(x) GCC_DIAG_DO_PRAGMA(GCC diagnostic x)
#define GCC_DIAG_OFF(x) GCC_DIAG_PRAGMA(push) \
	GCC_DIAG_PRAGMA(ignored GCC_DIAG_JOINSTR(-W, x))
#define GCC_DIAG_ON(x) GCC_DIAG_PRAGMA(pop)
class TestClass {
	public:
		int a;
GCC_DIAG_OFF(suggest-final-methods)
		virtual ~TestClass() { }
GCC_DIAG_ON(suggest-final-methods)
};
int main() {
TestClass a;
a.a = 0;
return a.a;
}
''', args : flags_fatal)
message('Wsuggest-final-methods: ' + wsuggest_final_methods.to_string())
conf.set('WSUGGEST_FINAL_METHODS', wsuggest_final_methods,
	description : 'Define if GCC diagnostic -Wsuggest-final-methods can be used')

wzero_as_null_pointer_constant = cxx.links('''
#define GCC_DIAG_STR(s) #s
#define GCC_DIAG_JOINSTR(x, y) GCC_DIAG_STR(x ## y)
#define GCC_DIAG_DO_PRAGMA(x) _Pragma (#x)
#define GCC_DIAG_PRAGMA(x) GCC_DIAG_DO_PRAGMA(GCC diagnostic x)
#define GCC_DIAG_OFF(x) GCC_DIAG_PRAGMA(push) \
	GCC_DIAG_PRAGMA(ignored GCC_DIAG_JOINSTR(-W, x))
#define GCC_DIAG_ON(x) GCC_DIAG_PRAGMA(pop)
static const char *f(const char *a) {
	return a;
}
int main() {
GCC_DIAG_OFF(zero-as-null-pointer-constant)
	return (f(0) == 0) ? 0 : 1;
GCC_DIAG_ON(zero-as-null-pointer-constant)
}
''', args : flags_fatal)
message('Wzero-as-null-pointer-constant: ' + wzero_as_null_pointer_constant.to_string())
conf.set('WZERO_AS_NULL_POINTER_CONSTANT', wzero_as_null_pointer_constant,
	description : 'Define if GCC diagnostic -Wzero-as-null-pointer-constant can be used')

wsuggest_attribute_const = cxx.links('''
#define GCC_DIAG_STR(s) #s
#define GCC_DIAG_JOINSTR(x, y) GCC_DIAG_STR(x ## y)
#define GCC_DIAG_DO_PRAGMA(x) _Pragma (#x)
#define GCC_DIAG_PRAGMA(x) GCC_DIAG_DO_PRAGMA(GCC diagnostic x)
#define GCC_DIAG_OFF(x) GCC_DIAG_PRAGMA(push) \
	GCC_DIAG_PRAGMA(ignored GCC_DIAG_JOINSTR(-W, x))
#define GCC_DIAG_ON(x) GCC_DIAG_PRAGMA(pop)
GCC_DIAG_OFF(suggest-attribute=const)
int f();
int f() {
	return 0;
}
int main() {
GCC_DIAG_OFF(suggest-attribute=const)
	return f();
GCC_DIAG_ON(suggest-attribute=const)
}
''', args : flags_fatal)
message('Wsuggest-attribute=const: ' + wzero_as_null_pointer_constant.to_string())
conf.set('WSUGGEST_ATTRIBUTE_CONST', wsuggest_attribute_const,
	description : 'Define if GCC diagnostic -Wsuggest-attribute=const can be used')

wsuggest_attribute_pure = cxx.links('''
#define GCC_DIAG_STR(s) #s
#define GCC_DIAG_JOINSTR(x, y) GCC_DIAG_STR(x ## y)
#define GCC_DIAG_DO_PRAGMA(x) _Pragma (#x)
#define GCC_DIAG_PRAGMA(x) GCC_DIAG_DO_PRAGMA(GCC diagnostic x)
#define GCC_DIAG_OFF(x) GCC_DIAG_PRAGMA(push) \
	GCC_DIAG_PRAGMA(ignored GCC_DIAG_JOINSTR(-W, x))
#define GCC_DIAG_ON(x) GCC_DIAG_PRAGMA(pop)
GCC_DIAG_OFF(suggest-attribute=pure)
int f();
int f() {
	return 0;
}
int main() {
GCC_DIAG_OFF(suggest-attribute=pure)
	return f();
GCC_DIAG_ON(suggest-attribute=pure)
}
''', args : flags_fatal)
message('Wsuggest-attribute=pure: ' + wzero_as_null_pointer_constant.to_string())
conf.set('WSUGGEST_ATTRIBUTE_PURE', wsuggest_attribute_pure,
	description : 'Define if GCC diagnostic -Wsuggest-attribute=pure can be used')

wgnu_statement_expression = cxx.links('''
#define GCC_DIAG_STR(s) #s
#define GCC_DIAG_JOINSTR(x, y) GCC_DIAG_STR(x ## y)
#define GCC_DIAG_DO_PRAGMA(x) _Pragma (#x)
#define GCC_DIAG_PRAGMA(x) GCC_DIAG_DO_PRAGMA(GCC diagnostic x)
#define GCC_DIAG_OFF(x) GCC_DIAG_PRAGMA(push) \
	GCC_DIAG_PRAGMA(ignored GCC_DIAG_JOINSTR(-W, x))
#define GCC_DIAG_ON(x) GCC_DIAG_PRAGMA(pop)
#include <cassert>
#define my_assert(a) GCC_DIAG_OFF(gnu-statement-expression) assert(a) GCC_DIAG_ON(gnu-statement-expression)
int main() {
my_assert(1);
return 0;
}
''', args : flags_fatal)
message('Wgnu-statement-expression: ' + wgnu_statement_expression.to_string())
conf.set('WGNU_STATEMENT_EXPRESSION', wgnu_statement_expression,
	description : 'Define if GCC diagnostic -Wgnu-statement-expression can be used in macro')

c11attr_noreturn = cxx.links('''
#include <cstdlib>
[[noreturn]] static void dummy();
static void
dummy()
{ exit(0); }
int main() {
	dummy();
	return 0;
}
''', args : flags_fatal)
message('[[noreturn]]: ' + c11attr_noreturn.to_string())
conf.set('HAVE_C11ATTRIBUTE_NORETURN', c11attr_noreturn,
	description : 'Define if [[noreturn]] can be used')

attribute_noreturn = cxx.links('''
#include <cstdlib>
__attribute__ ((noreturn)) static void dummy();
static void
dummy()
{ exit(0); }
int main() {
	dummy();
	return 0;
}
''', args : flags_fatal)
message('__attribute__ ((noreturn)): ' + attribute_noreturn.to_string())
conf.set('HAVE_C11ATTRIBUTE_NORETURN', attribute_noreturn,
	description : 'Define if __attribute__ ((noreturn)) can be used')

attr_fallthrough = cxx.links('''
int main() {
	int i = 0;
	switch(i) {
		case 0:
			--i;
			[[fallthrough]];
		default:
			return i + 1;
	}
}
''', args : flags_fatal)
message('[[fallthrough]]: ' + attr_fallthrough.to_string())
conf.set('HAVE_ATTRIBUTE_FALLTHROUGH', attr_fallthrough,
	description : 'Define if [[fallthrough]] can be used')

attr_gnufallthrough = cxx.links('''
int main() {
	int i = 0;
	switch(i) {
		case 0:
			--i;
			[[gnu::fallthrough]];
		default:
			return i + 1;
	}
}
''', args : flags_fatal)
message('[[gnu::fallthrough]]: ' + attr_gnufallthrough.to_string())
conf.set('HAVE_ATTRIBUTE_GNUFALLTHROUGH', attr_gnufallthrough,
	description : 'Define if [[gnu::fallthrough]] can be used')

attr_afallthrough = cxx.links('''
int main() {
	int i = 0;
	switch(i) {
		case 0:
			--i;
			__attribute__ ((fallthrough));
		default:
			return i + 1;
	}
}
''', args : flags_fatal)
message('__attribute__ ((fallthrough)) ' + attr_afallthrough.to_string())
conf.set('HAVE_ATTRIBUTE_AFALLTHROUGH', attr_afallthrough,
	description : 'Define if __attribute__ ((fallthrough)) can be used')

attr_signal = cxx.links('''
#include <csignal>
__attribute__ ((signal)) void my_sig_handler(int sig);
class my_int {
friend __attribute__ ((signal)) void my_sig_handler(int sig);
private:	static int i;
		static void set(int s) { i=s; }
};
int my_int::i;
void my_sig_handler(int sig)
{ my_int::set(sig); }
int main() {
	signal(SIGTERM, my_sig_handler);
	return 0;
}
''', args : flags_fatal)
message('__attribute__ ((signal)) ' + attr_signal.to_string())
conf.set('HAVE_ATTRIBUTE_SIGNAL', attr_signal,
	description : 'Define if __attribute__ ((signal)) can be used')

attr_const = cxx.links('''
class b {
public:
	static int c(int i) __attribute__ ((const)) { return i; }
};
int a(int i) __attribute__ ((const));
int a(int i) { b x; return x.c(i); }
int main() {
	return !((a(4) == 4) && (a(5) == 5));
}
''', args : flags_fatal)
message('__attribute__ ((const)) ' + attr_const.to_string())
conf.set('HAVE_ATTRIBUTE_CONST', attr_const,
	description : 'Define if __attribute__ ((const)) can be used')

attr_pure = cxx.links('''
class b {
public:
	static int c(int *i) __attribute__ ((pure)) { return *i; }
};
__attribute__ ((pure)) int a(int i);
int a(int i) { b x; return x.c(&i); }
int main() {
	return !((a(4) == 4) && (a(5) == 5));
}
''', args : flags_fatal)
message('__attribute__ ((pure)) ' + attr_pure.to_string())
conf.set('HAVE_ATTRIBUTE_PURE', attr_pure,
	description : 'Define if __attribute__ ((pure)) can be used')

pure_for_const_virtuals = get_option('pure-for-const-virtuals')
result += [ 'pure-for-const-virtuals=' + pure_for_const_virtuals.to_string() ]
conf.set('USE_PURE_FOR_CONST_VIRTUALS', pure_for_const_virtuals,
	description : 'Define if __attribute__ ((pure)) can be used for const virtuals')

const_for_const_virtuals = get_option('const-for-const-virtuals')
result += [ 'const-for-const-virtuals=' + const_for_const_virtuals.to_string() ]
conf.set('USE_CONST_FOR_CONST_VIRTUALS', const_for_const_virtuals,
	description : 'Define if __attribute__ ((const)) can be used for const virtuals')

attr_nonnull_ = cxx.links('''
class b {
public:
	__attribute__ ((nonnull)) int c(int *i) { return *i; }
};
int a(int *i) __attribute__ ((nonnull));
int a(int *i) { b x; return x.c(i); }
int main() {
	int i = 4, j = 5;
	return !((a(&i) == 4) && (a(&j) == 5));
}
''', args : flags_fatal)
message('__attribute__ ((nonnull)) ' + attr_nonnull_.to_string())
conf.set('HAVE_ATTRIBUTE_NONNULL_', attr_nonnull_,
	description : 'Define if __attribute__ ((nonnull)) can be used')

attr_nonnull = cxx.links('''
class b {
public:
	__attribute__ ((nonnull (1, 3))) static int c(int *i, int *j, int *k) {
		return (j ? ((*i) - (*k)) : ((*i) + (*k)));
	}
	__attribute__ ((nonnull (2, 4))) int d(int *i, int *j, int *k) {
		return c(i,j,k);
	}
};
__attribute__ ((nonnull (1, 3))) int a(int *i, int *j, int *k);
int a(int *i, int *j, int *k) {
	b x; return x.d(i, j, k) + x.d(i, 0, k);
}
int main() {
	int i = 1;
	b x;
	return !((x.d(&i, 0, &i) == 2) && (x.d(&i, 0, &i) == 2) && (a(&i, 0, &i) == 4));
}
''', args : flags_fatal)
message('__attribute__ ((nonnull (list))) ' + attr_nonnull.to_string())
conf.set('HAVE_ATTRIBUTE_NONNULL', attr_nonnull,
	description : 'Define if __attribute__ ((nonnull (list))) can be used')

nullptr = cxx.links('''
static const char *a = nullptr;
int main() {
	const char *b(nullptr);
	return !((a == b) && (b == nullptr));
}
''', args : flags_dialect)
message('nullptr: ' + nullptr.to_string())
conf.set('HAVE_NULLPTR', nullptr,
	description : 'Define if C++ dialect has nullptr type')

constexpr = cxx.links('''
class T {
public:
	static constexpr const int a = 5;
};
const int T::a;
int main() {
	return (T::a != 5);
}
''', args : flags_dialect)
message('constexpr: ' + constexpr.to_string())
conf.set('HAVE_CONSTEXPR', constexpr,
	description : 'Define if C++ dialect has constexpr modifier')

override = cxx.links('''
class T {
public:
	virtual int foo() {return 1;}
};
class S : public T {
public:
	int foo() override {return 0;}
};
int main() {
	S a;
	T *b(&a);
	return b->foo();
}
''', args : flags_dialect)
message('override: ' + override.to_string())
conf.set('HAVE_OVERRIDE', override,
	description : 'Define if C++ dialect has override modifier')

final = cxx.links('''
class T final {
public:
	virtual int foo() final {return 0;}
};
int main() {
	T b;
	return b.foo();
}
''', args : flags_dialect)
message('final: ' + final.to_string())
conf.set('HAVE_FINAL', final,
	description : 'Define if C++ dialect has final modifier')

delete = cxx.links('''
#include <utility>
class T {
private:
	T(const T& s) = delete;
	T& operator=(const T& s) = delete;
public:
	T() { }
};
int main() {
	T a;
	return 0;
}
''', args : flags_dialect)
message('delete: ' + delete.to_string())
conf.set('HAVE_DELETE', delete,
	description : 'Define if C++ dialect supports assigning delete')

noexcept = cxx.links('''
class T {
public:
	int s;
	T() noexcept : s(0) {
	}
};
int main() {
	return T().s;
}
''', args : flags_dialect)
message('noexcept: ' + noexcept.to_string())
conf.set('HAVE_NOEXCEPT', noexcept,
	description : 'Define if C++ dialect has noexcept')

have_move = cxx.links('''
#include <utility>
class T {
public:
	T() { }
	~T() { }
	T(T&&) { }
	T& operator=(T&&) { return *this; }
	int ret() { return 0; }
};
int main() {
	T a, b;
	T c = std::move(a);
	b = std::move(c);
	return b.ret();
}
''', args : flags_dialect)
message('move constructor: ' + have_move.to_string())
conf.set('HAVE_MOVE', have_move,
	description : 'Define if C++ dialect supports assigning default move constructor')

have_array_class = cxx.links('''
#include <array>
int main() {
	std::array<int, 5> a;
	a.fill(4);
	return (a[4] == 4) ? 0 : 1;
}
''', args : flags_dialect)
message('array class: ' + have_array_class.to_string())
conf.set('HAVE_ARRAY_CLASS', have_array_class,
	description : 'Define if STL has array class')

have_forward_list = cxx.links('''
#include <forward_list>
int main() {
	std::forward_list<int> a;
	a.insert_after(a.before_begin(), 5);
	return (*(a.begin()) == 5) ? 0 : 1;
}
''', args : flags_dialect)
message('forward_list: ' + have_forward_list.to_string())
conf.set('HAVE_FORWARD_LIST', have_forward_list,
	description : 'Define if STL has forward_list')

have_unordered_set = cxx.links('''
#include <unordered_set>
int main() {
	std::unordered_set<int> a;
	a.insert(5);
	return (a.count(5) == 1) ? 0 : 1;
}
''', args : flags_dialect)
message('unordered_set: ' + have_unordered_set.to_string())
conf.set('HAVE_UNORDERED_SET', have_unordered_set,
	description : 'Define if STL has unordered_set')

have_unordered_map = cxx.links('''
#include <unordered_map>
int main() {
	std::unordered_map<int, int> a;
	a[5] = 4;
	return (a[5] == 4) ? 0 : 1;
}
''', args : flags_dialect)
message('unordered_map: ' + have_unordered_map.to_string())
conf.set('HAVE_UNORDERED_MAP', have_unordered_map,
	description : 'Define if STL has unordered_map')

have_emplace_back = cxx.links('''
#include <utility>
#include <vector>
int main() {
	typedef std::pair<int, int> mypair;
	std::vector<mypair> a;
	a.push_back(mypair(4, 4));
	a.emplace_back(5, 5);
	std::vector<bool> b;
	b.push_back(true);
	b.emplace_back(true);
	return ((a.size() == 2) && (b.size() == 2)) ? 0 : 1;
}
''', args : flags_dialect)
message('emplace_back: ' + have_emplace_back.to_string())
conf.set('HAVE_EMPLACE_BACK', have_emplace_back,
	description : 'Define if STL has emplace_back')

have_emplace = cxx.links('''
#include <set>
#include <utility>
int main() {
	typedef std::pair<int, int> mypair;
	std::set<mypair> a;
	a.insert(mypair(4, 4));
	a.emplace(5, 5);
	return (a.size() == 2) ? 0 : 1;
}
''', args : flags_dialect)
message('emplace: ' + have_emplace.to_string())
conf.set('HAVE_EMPLACE', have_emplace,
	description : 'Define if STL has emplace')

sse2 = get_option('sse2')
support_sse2 = false
sse2_msg = ''
if sse2 == 'auto'
	sse2_msg = ' (auto)'
	support_sse2 = cxx.compiles('''
#include <emmintrin.h>
#ifndef __SSE2__
#error "SSE2 not available"
#endif
''', args : flags_dialect)
elif sse2 == 'true'
	support_sse2 = true
endif
message('sse2: ' + support_sse2.to_string() + sse2_msg)
conf.set('SUPPORT_SSE2', support_sse2,
	description : 'Define if support for sse2 is wanted')

sqlite_dep = []
with_sqlite = false
want_sqlite = get_option('sqlite')
sqlite_msg = ''
if want_sqlite == 'auto'
	sqlite_msg = ' (auto)'
endif
if want_sqlite != 'false'
	sqlite_only_dep = dependency('sqlite3', required : false)
	if sqlite_only_dep.found()
		sqlite_dep = [sqlite_only_dep]
		with_sqlite = true
		want_sqlite = 'true'
	elif want_sqlite == 'true'
		error('sqlite required by option but not found')
	else
		want_sqlite = 'false'
	endif
endif
result += [ 'sqlite=' + want_sqlite + sqlite_msg ]
conf.set('WITH_SQLITE', with_sqlite,
	description: 'Define if cache method sqlite is wanted')

protobuf_dep = []
with_protobuf = false
want_protobuf = get_option('protobuf')
protobuf_msg = ''
if want_protobuf == 'auto'
	protobuf_msg = ' (auto)'
endif
if want_protobuf != 'false'
	protobuf_only_dep = dependency('protobuf', required : false)
	if protobuf_only_dep.found()
		protobuf_dep = [protobuf_only_dep]
		with_protobuf = true
		want_protobuf = 'true'
	elif want_protobuf == true
		error('protobuf required by option but not found')
	else
		want_protobuf = 'false'
	endif
endif
result += [ 'protobuf=' + want_protobuf + protobuf_msg ]
conf.set('WITH_PROTOBUF', with_protobuf,
	description: 'Define if cache method protobuf is wanted')

proto_src = []
if with_protobuf
	proto_src = custom_target('proto',
		input : files(join_paths('src', 'output', 'eix.proto')),
		output : ['protobuf-generated.h'], # ['eix.pb.cc', 'eix.pb.h'],
		command : [sh, '-c', '''
		p=${1%?eix.proto}
		d=$2/output
		test -d "$d" || mkdir "$d"
		protoc --cpp_out="$d" --proto_path="$p" eix.proto && : >"$3"
		''', 'sh', '@INPUT@', '@OUTDIR@', '@OUTPUT@'])
endif

portdir_cache_method = get_option('portdir-cache-method')
result += [ 'portdir-cache-method=' + portdir_cache_method ]
subst.set('PORTDIR_CACHE_METHOD', portdir_cache_method)
conf.set_quoted('PORTDIR_CACHE_METHOD', portdir_cache_method,
	description : 'Default PORTDIR_CACHE_METHOD')

portdir_default = get_option('portdir-default')
result += [ 'portdir-default=' + portdir_default ]
conf.set_quoted('PORTDIR_DEFAULT', portdir_default,
	description : 'Default PORTDIR')

eprefix_default = get_option('eprefix-default')
eprefix_default_msg = ''
if eprefix_default == 'auto'
	eprefix_default = ''
	r = run_command('portageq', 'envvar', 'EPREFIX')
	if r.returncode() == 0
		eprefix_default = r.stdout().strip()
		eprefix_default_msg = ' (auto)'
	endif
endif
result += [ 'eprefix-default=' + eprefix_default + eprefix_default_msg ]
conf.set_quoted('EPREFIX_DEFAULT', eprefix_default,
	description : 'Default EPREFIX')

sh_shebang = get_option('sh-shebang')
sh_shebang_msg = ''
if sh_shebang == 'auto'
	sh_shebang = ''
	r = run_command(sh, '-c', 'command', '-v', 'sh')
	if r.returncode() == 0
		sh_shebang = r.stdout().strip()
		sh_shebang_msg = ' (auto)'
	endif
endif
if sh_shebang == ''
	sh_shebang = '/usr/bin/env sh'
	sh_shebang_msg = ' (fallback)'
endif
result += [ 'sh-shebang=' + sh_shebang ]
subst.set('SH_SHEBANG', sh_shebang)

eix_prefix_default = get_option('eix-prefix-default')
result += [ 'eix-prefix-default=' + eix_prefix_default ]
conf.set_quoted('EIX_PREFIX_DEFAULT', eix_prefix_default,
	description : 'Default EIX_PREFIX')

root_default = get_option('root-default')
result += [ 'root-default=' + root_default ]
conf.set_quoted('ROOT_DEFAULT', root_default,
	description : 'Default ROOT')

portage_rootpath = get_option('portage-rootpath')
result += [ 'portage-rootpath=' + portage_rootpath ]
conf.set_quoted('PORTAGE_ROOTPATH_DEFAULT', portage_rootpath,
	description : 'Default PORTAGE_ROOTPATH')

default_arch = get_option('default-arch')
result += [ 'default-arch=' + default_arch ]
conf.set_quoted('ARCH_DEFAULT', default_arch,
	description : 'Default DEFAULT_ARCH')

eix_user = get_option('eix-user')
result += [ 'eix-user=' + eix_user ]
conf.set_quoted('EIX_USER', eix_user,
	description : 'Default EIX_USER')

eix_uid = get_option('eix-uid')
result += [ 'eix-uid=' + eix_uid ]
conf.set_quoted('EIX_UID', eix_uid,
	description : 'Default EIX_UID')

eix_group = get_option('eix-group')
result += [ 'eix-group=' + eix_group ]
conf.set_quoted('EIX_GROUP', eix_group,
	description : 'Default EIX_GROUP')

eix_gid = get_option('eix-gid')
result += [ 'eix-gid=' + eix_gid ]
conf.set_quoted('EIX_GID', eix_gid,
	description : 'Default EIX_GID')

if get_option('swap-remote')
	remote_file1 = get_option('remote-file2')
	remote_file2 = get_option('remote-file1')
else
	remote_file1 = get_option('remote-file1')
	remote_file2 = get_option('remote-file2')
endif
result += [ 'remote-file1=' + remote_file1 ]
result += [ 'remote-file2=' + remote_file2 ]
subst.set('DEFAULT_REMOTE_PATH1', remote_file1)
subst.set('DEFAULT_REMOTE_PATH2', remote_file2)

always_accept_keywords = get_option('always-accept-keywords')
result += [ 'always-accept-keywords=' + always_accept_keywords.to_string() ]
conf.set_quoted('ALWAYS_ACCEPT_KEYWORDS_DEFAULT', always_accept_keywords.to_string(),
	description : 'Default ALWAYS_ACCEPT_KEYWORDS')

dep_default = get_option('dep-default')
result += [ 'dep-default=' + dep_default.to_string() ]
conf.set_quoted('DEP_DEFAULT', dep_default.to_string(),
	description : 'Default DEP')

src_uri_default = get_option('src-uri-default')
result += [ 'src-uri-default=' + src_uri_default.to_string() ]
conf.set_quoted('SRC_URI_DEFAULT', src_uri_default.to_string(),
	description : 'Default SRC_URI')

required_use_default = get_option('required-use-default')
result += [ 'required-use-default=' + required_use_default.to_string() ]
conf.set_quoted('REQUIRED_USE_DEFAULT', required_use_default.to_string(),
	description : 'Default REQUIRED_USE')

zsh_completion_path = get_option('zsh-completion')
zsh_completions = []
if zsh_completion_path != ''
	zsh_completion_path = join_paths(prefix, zsh_completion_path)
	foreach file : files(
		join_paths('zsh', '_eix-header'),
		join_paths('zsh', '_eix-installed'),
		join_paths('zsh', '_eix-installed-after'),
		join_paths('zsh', '_eix-layman'),
		join_paths('zsh', '_eix-remote'),
		join_paths('zsh', '_eix-sync'),
		join_paths('zsh', '_eix-test-obsolete'),
	)
		install_data(file, install_dir : zsh_completion_path)
	endforeach
	zsh_completions = [
		'_eix'
	]
endif
result += [ 'zsh-completion=' + zsh_completion_path ]

extra_doc = get_option('extra-doc')
result += [ 'extra-doc=' + extra_doc.to_string() ]

sed_get = [ '-n', '-e',
	's/^.*static * CONSTEXPR *.* current *= *\([0-9]*\) *;.*$/\1/p',
	'--',
]
f = join_paths('src', 'database', 'header.h')
database_version = ''
r = run_command(sed, sed_get, files(f))
if r.returncode() == 0
	database_version = r.stdout().strip()
endif
if database_version == ''
	error('failed to get current database version from ' + f)
endif
subst.set('CURRENT', database_version)

f = join_paths('src', 'output', 'print-xml.h')
xml_version = ''
r = run_command(sed, sed_get, files(f))
if r.returncode() == 0
	xml_version = r.stdout().strip()
endif
if xml_version == ''
	error('failed to get currrent xml version from ' + f)
endif
subst.set('CURRENTXML', xml_version)

eix_cachefile = '/var/cache/eix/portage.eix'
subst.set('EIX_CACHEFILE', eix_cachefile)
conf.set_quoted('EIX_CACHEFILE', eix_cachefile,
	description : 'Location of the cachefile')

eix_previous = '/var/cache/eix/previous.eix'
subst.set('EIX_PREVIOUS', eix_previous)
conf.set_quoted('EIX_PREVIOUS', eix_previous,
	description : 'Location of the previous cachefile')

eix_remotecachefile1 = '/var/cache/eix/remote.eix'
subst.set('EIX_REMOTECACHEFILE1', eix_remotecachefile1)
conf.set_quoted('EIX_REMOTECACHEFILE1', eix_remotecachefile1,
	description : 'Location of the remote cachefile 1')

eix_remotecachefile2 = '/var/cache/eix/remote2.eix'
subst.set('EIX_REMOTECACHEFILE2', eix_remotecachefile2)
conf.set_quoted('EIX_REMOTECACHEFILE2', eix_remotecachefile2,
	description : 'Location of the remote cachefile 2')

eix_remotearchive1 = '/var/cache/eix/remote.tar.bz2'
subst.set('EIX_REMOTEARCHIVE1', eix_remotearchive1)
conf.set_quoted('EIX_REMOTEARCHIVE1', eix_remotearchive1,
	description : 'Location of the local remote cache arhive 1')

eix_remotearchive2 = '/var/cache/eix/remote2.tar.bz2'
subst.set('EIX_REMOTEARCHIVE2', eix_remotearchive2)
conf.set_quoted('EIX_REMOTEARCHIVE2', eix_remotearchive2,
	description : 'Location of the local remote cache arhive 2')

levenshtein_distance = '2'
subst.set('LEVENSHTEIN_DISTANCE_DEFAULT', levenshtein_distance)
conf.set_quoted('LEVENSHTEIN_DISTANCE_DEFAULT', levenshtein_distance,
	description : 'Default Levenshtein distance for matches (as string)')

configure_file(
	output : 'config.h',
	configuration : conf,
)

configure_file(
	output : 'config_vers.h',
	configuration : vers,
)

header_lib = [ static_library('header',
	join_paths('src', 'database', 'io.cc'),
	join_paths('src', 'database', 'io_header.cc'),
	join_paths('src', 'database', 'header.cc'),
	include_directories : incdir,
) ]

database_lib = [ static_library('database',
	join_paths('src', 'database', 'header_portage.cc'),
	join_paths('src', 'database', 'io_portage.cc'),
	join_paths('src', 'database', 'package_reader.cc'),
	include_directories : incdir,
) ]
database_lib += header_lib

stringutils_lib = [ static_library('stringutils',
	join_paths('src', 'eixTk', 'compare.cc'),
	join_paths('src', 'eixTk', 'formated.cc'),
	join_paths('src', 'eixTk', 'stringutils.cc'),
	include_directories : incdir,
) ]

sysutils_lib = [ static_library('sysutils',
	join_paths('src', 'eixTk', 'sysutils.cc'),
	include_directories : incdir,
) ]

filenames_lib = [ static_library('filenames',
	join_paths('src', 'eixTk', 'filenames.cc'),
	include_directories : incdir,
) ]

utils_lib = [ static_library('utils',
	join_paths('src', 'eixTk', 'utils.cc'),
	include_directories : incdir,
) ]

varsreader_lib = [ static_library('varsreader',
	join_paths('src', 'eixTk', 'varsreader.cc'),
	include_directories : incdir,
) ]
varsreader_lib += filenames_lib
varsreader_lib += utils_lib

parseerror_lib = [ static_library('parseerror',
	join_paths('src', 'eixTk', 'parseerror.cc'),
	include_directories : incdir,
) ]

masklist_lib = [ static_library('masklist',
	join_paths('src', 'eixTk', 'stringlist.cc'),
	join_paths('src', 'portage', 'mask.cc'),
	join_paths('src', 'portage', 'mask_list.cc'),
	include_directories : incdir,
) ]

argsreader_lib = [ static_library('argsreader',
	join_paths('src', 'eixTk', 'argsreader.cc'),
	include_directories : incdir,
) ]

eixtk_lib = [ static_library('eixtk',
	join_paths('src', 'eixTk', 'ansicolor.cc'),
	join_paths('src', 'eixTk', 'regexp.cc'),
	include_directories : incdir,
) ]
eixtk_lib += argsreader_lib
eixtk_lib += parseerror_lib
eixtk_lib += stringutils_lib
eixtk_lib += sysutils_lib
eixtk_lib += varsreader_lib

if jumbo_build
eixrc_lib = [ static_library('eixrc',
	join_paths('src', 'eixrc', 'eixrc.cc'),
	join_paths('src', 'eixrc', 'global.cc'),
	join_paths('src', 'eixrc', 'globals.cc'),
	include_directories : incdir,
) ]
else
eixrc_lib = [ static_library('eixrc',
	join_paths('src', 'eixrc', 'eixrc.cc'),
	join_paths('src', 'eixrc', 'global.cc'),
	join_paths('src', 'eixrc', 'global1.cc'),
	join_paths('src', 'eixrc', 'global2.cc'),
	join_paths('src', 'eixrc', 'global3.cc'),
	join_paths('src', 'eixrc', 'global4.cc'),
	join_paths('src', 'eixrc', 'global5.cc'),
	join_paths('src', 'eixrc', 'global6.cc'),
	include_directories : incdir,
) ]
endif

depend_lib = [ static_library('depend',
	join_paths('src', 'portage', 'depend.cc'),
	include_directories : incdir,
) ]

overlay_lib = [ static_library('overlay',
	join_paths('src', 'portage', 'overlay.cc'),
	include_directories : incdir,
) ]

basicversion_lib = [ static_library('basicversion',
	join_paths('src', 'portage', 'basicversion.cc'),
	include_directories : incdir,
) ]

version_lib = [ static_library('version',
	join_paths('src', 'portage', 'extendedversion.cc'),
	join_paths('src', 'portage', 'keywords.cc'),
	join_paths('src', 'portage', 'package.cc'),
	join_paths('src', 'portage', 'version.cc'),
	include_directories : incdir,
) ]
version_lib += basicversion_lib

portage_lib = [ static_library('portage',
	join_paths('src', 'portage', 'conf', 'portagesettings.cc'),
	join_paths('src', 'portage', 'conf', 'cascadingprofile.cc'),
	join_paths('src', 'portage', 'eapi.cc'),
	join_paths('src', 'portage', 'extendedversion_bin.cc'),
	join_paths('src', 'portage', 'instversion.cc'),
	join_paths('src', 'portage', 'package_best.cc'),
	join_paths('src', 'portage', 'packagesets.cc'),
	join_paths('src', 'portage', 'vardbpkg.cc'),
	join_paths('src', 'portage', 'packagetree.cc'),
	join_paths('src', 'portage', 'overlay_bin.cc'),
	join_paths('src', 'portage', 'set_stability.cc'),
	include_directories : incdir,
) ]
portage_lib += masklist_lib
portage_lib += depend_lib
portage_lib += version_lib
portage_lib += overlay_lib

gencache_src = custom_target('cache_map',
	input : files(join_paths('src', 'cache', 'generate_cachemap.sh')),
	output : [ 'cache_map.cc' ],
	command : [ sh, '@INPUT@' ],
	capture : true,
)
cache_lib = [ static_library('cache',
	join_paths('src', 'cache', 'cachetable.cc'),
	join_paths('src', 'cache', 'common', 'assign_reader.cc'),
	join_paths('src', 'cache', 'common', 'ebuild_exec.cc'),
	join_paths('src', 'cache', 'common', 'flat_reader.cc'),
	join_paths('src', 'cache', 'common', 'selectors.cc'),
	join_paths('src', 'cache', 'base.cc'),
	join_paths('src', 'cache', 'eixcache', 'eixcache.cc'),
	join_paths('src', 'cache', 'metadata', 'metadata.cc'),
	join_paths('src', 'cache', 'parse', 'parse.cc'),
	join_paths('src', 'cache', 'sqlite', 'sqlite.cc'),
	join_paths('src', 'eixTk', 'md5.cc'),
	gencache_src,
	include_directories : incdir,
) ]

cli_lib = [ static_library('cli',
	join_paths('src', 'various', 'cli.cc'),
	include_directories : incdir,
) ]

drop_permissions_lib = [ static_library('drop_permissions',
	join_paths('src', 'various', 'drop_permissions.cc'),
	include_directories : incdir,
) ]

percentage_lib = [ static_library('percentage',
	join_paths('src', 'eixTk', 'percentage.cc'),
	join_paths('src', 'eixTk', 'statusline.cc'),
	include_directories : incdir,
) ]

printformats_lib = [ static_library('printformats',
	join_paths('src', 'output', 'eix-proto.cc'),
	join_paths('src', 'output', 'print-proto.cc'),
	join_paths('src', 'output', 'print-xml.cc'),
	proto_src,
	include_directories : incdir,
) ]

outputstring_lib = [ static_library('outputstring',
	join_paths('src', 'eixTk', 'outputstring.cc'),
	join_paths('src', 'eixTk', 'stringlist_output.cc'),
	include_directories : incdir,
) ]

output_lib = [ static_library('output',
	join_paths('src', 'portage', 'version_output.cc'),
	join_paths('src', 'output', 'formatstring.cc'),
	join_paths('src', 'output', 'formatstring-print.cc'),
	include_directories : incdir,
) ]
output_lib += outputstring_lib

search_lib = [ static_library('search',
	join_paths('src', 'search', 'levenshtein.cc'),
	join_paths('src', 'search', 'algorithms.cc'),
	join_paths('src', 'search', 'matchtree.cc'),
	join_paths('src', 'search', 'packagetest.cc'),
	join_paths('src', 'search', 'packagetest_default.cc'),
	join_paths('src', 'search', 'nowarn.cc'),
	include_directories : incdir,
) ]

common_lib = [ static_library('common',
	join_paths('src', 'eixrc', 'eixrc_portage.cc'),
	include_directories : incdir,
) ]
common_lib += drop_permissions_lib
common_lib += eixtk_lib
common_lib += database_lib
common_lib += portage_lib
common_lib += eixrc_lib

eix_only_lib = [ static_library('eix',
	join_paths('src', 'eix.cc'),
	join_paths('src', 'eixTk', 'ansicolor_print.cc'),
	include_directories : incdir,
) ]
eix_only_lib += cli_lib
eix_only_lib += printformats_lib
eix_only_lib += search_lib

diff_only_lib = [ static_library('diff',
	join_paths('src', 'eix-diff.cc'),
	include_directories : incdir,
) ]

update_only_lib = [ static_library('update',
	join_paths('src', 'eix-update.cc'),
	include_directories : incdir,
) ]
update_only_lib += percentage_lib
update_only_lib += cache_lib

masked_packages_lib = [ static_library('masked_packages',
	join_paths('src', 'masked-packages.cc'),
	include_directories : incdir,
) ]
masked_packages_lib += parseerror_lib

versionsort_lib = [ static_library('versionsort',
	join_paths('src', 'versionsort.cc'),
	include_directories : incdir,
) ]

eix_header_lib = [ static_library('eix_header',
	join_paths('src', 'eix-header.cc'),
	include_directories : incdir,
) ]

eix_drop_permissions_lib = [ static_library('eix_drop_permissions',
	join_paths('src', 'eix-drop-permissions.cc'),
	include_directories : incdir,
) ]

main_output_src = files(
	join_paths('src', 'main', 'main_output.cc'),
)

main_eix_src = files(
	join_paths('src', 'main', 'main_eix.cc'),
)

main_diff_src = files(
	join_paths('src', 'main', 'main_diff.cc'),
)

inst_link = []
inst_link_tools = []
separate_tools = get_option('separate-tools')
separate_binaries = get_option('separate-binaries')
separate_update = get_option('separate-update')
separate_update_msg = ''
if separate_binaries and not separate_update
	separate_update = true
	separate_update_msg = ' (forced by separate-binaries)'
endif
result += [ 'separate-tools=' + separate_tools.to_string() ]
result += [ 'separate-binaries=' + separate_binaries.to_string() ]
result += [ 'separate-update=' + separate_update.to_string() + separate_update_msg ]
main_all_link_with = []
eix_update_link_with = []
if separate_tools
	masked_packages_link_with = masked_packages_lib
	masked_packages_link_with += argsreader_lib
	masked_packages_link_with += utils_lib
	masked_packages_link_with += version_lib
	masked_packages_link_with += stringutils_lib
	masked_packages_link_with += masklist_lib
	executable('masked-packages',
		join_paths('src', 'main', 'main_masked_packages.cc'),
		link_with : masked_packages_link_with,
		include_directories : incdir,
		install : true,
	)
	versionsort_link_with = versionsort_lib
	versionsort_link_with += basicversion_lib
	versionsort_link_with += stringutils_lib
	executable('versionsort',
		join_paths('src', 'main', 'main_versionsort.cc'),
		link_with : versionsort_link_with,
		include_directories : incdir,
		install : true,
	)
	eix_header_link_with = eix_header_lib
	eix_header_link_with += stringutils_lib
	eix_header_link_with += filenames_lib
	eix_header_link_with += utils_lib
	eix_header_link_with += header_lib
	eix_header_link_with += depend_lib
	eix_header_link_with += overlay_lib
	eix_header_link_with += sysutils_lib
	executable('eix-header',
		join_paths('src', 'main', 'main_overlay.cc'),
		link_with : eix_header_link_with,
		include_directories : incdir,
		install : true,
	)
	eix_drop_permissions_link_with = eix_drop_permissions_lib
	eix_drop_permissions_link_with += stringutils_lib
	eix_drop_permissions_link_with += eixrc_lib
	eix_drop_permissions_link_with += drop_permissions_lib
	eix_drop_permissions_link_with += sysutils_lib
	eix_drop_permissions_link_with += varsreader_lib
	executable('eix-drop-permissions',
		join_paths('src', 'main', 'main_drop_permissions.cc'),
		link_with : eix_drop_permissions_link_with,
		include_directories : incdir,
		install : true,
	)
	main_all_src = files(
		join_paths('src', 'main', 'main_all.cc'),
	)
	eix_update_src = files(
		join_paths('src', 'main', 'main_update.cc'),
	)
else
	inst_link_tools = [
		'masked-packages',
		'versionsort',
		'eix-header',
		'eix-drop-permissions',
	]
	main_all_src = files(
		join_paths('src', 'main', 'main_all_tools.cc'),
	)
	main_all_link_with += masked_packages_lib
	main_all_link_with += versionsort_lib
	main_all_link_with += masked_packages_lib
	main_all_link_with += versionsort_lib
	main_all_link_with += eix_header_lib
	main_all_link_with += eix_drop_permissions_lib
	eix_update_src = files(
		join_paths('src', 'main', 'main_update_tools.cc'),
	)
	eix_update_link_with += masked_packages_lib
	eix_update_link_with += versionsort_lib
	eix_update_link_with += eix_header_lib
	eix_update_link_with += eix_drop_permissions_lib
endif
eix_update_link_with += update_only_lib
eix_update_link_with += common_lib
eix_dep = sqlite_dep
eix_update_link = 'eix'
if separate_binaries or separate_update
	eix_update_link = 'eix'
	executable('eix-update', eix_update_src,
		dependencies : eix_dep,
		link_with : eix_update_link_with,
		include_directories : incdir,
		install : true,
	)
	eix_dep = []
endif
foreach l : inst_link_tools
	inst_link += [
		[ eix_update_link, l ],
	]
endforeach
eix_link_with = eix_only_lib
eix_link_with += main_all_link_with
if separate_binaries
	eix_src = main_eix_src
	eix_diff_link_with = diff_only_lib
	eix_diff_link_with += output_lib
	eix_diff_link_with += common_lib
	executable('eix-diff', main_diff_src,
		link_with : eix_diff_link_with,
		include_directories : incdir,
		install : true,
	)
else
	inst_link += [
		[ 'eix', 'eix-diff' ],
	]
	eix_link_with += diff_only_lib
	if separate_update
		eix_src = main_output_src
	else
		eix_src = main_all_src
		inst_link += [
			[ 'eix', 'eix-update' ],
		]
		eix_link_with += update_only_lib
	endif
endif
eix_link_with += output_lib
eix_link_with += common_lib
eix_dep += protobuf_dep
executable('eix', eix_src,
	dependencies : eix_dep,
	link_with : eix_link_with,
	include_directories : incdir,
	install : true,
)

bin_scripts = [
	'eix-etcat',
	'eix-functions',
	'eix-installed',
	'eix-installed-after',
	'eix-layman',
	'eix-postsync',
	'eix-remote',
	'eix-sync',
	'eix-test-obsolete',
]

sed_args = [
	'-e', 's,/dev/null,@DEV_NULL@,g',
	'-e', 's,\./eix-functions\.sh\.in,\'@BINDIR@/eix-functions.sh\',g',
	'-e', 's,\./eix-drop-permissions,\'@BINDIR@/eix-drop-permissions\',g',
	'-e', 's,\./eix-postsync\.sh\.in,\'@BINDIR@/eix-postsync\',g',
	'-e', 's,\./eix-remote\.sh\.in,\'@BINDIR@/eix-remote\',g',
	'-e', 's,^#!/usr/bin/env sh$,#!@SH_SHEBANG@,g',
	'--',
]
eixdatadir = join_paths(datadir, 'eix')
eixdatafile = join_paths(eixdatadir, 'eix-functions')
foreach file : bin_scripts
	file_sed = file + '.sh.subst'
	path_in = files(join_paths('src', file + '.sh.in'))
	configure_file(
		input : path_in,
		output : file_sed,
		command : [ sed, sed_args, path_in ],
		capture : true,
	)
	path_sed = join_paths(builddir, file_sed)
	c = configure_file(
		input : path_sed,
		output : file,
		configuration : subst,
	)
	if file != 'eix-functions'
		install_data(c, install_dir : bindir)
	else
		install_data(c, install_dir : eixdatadir,
			install_mode : 'rw-r--r--')
	endif
endforeach
c = configure_file(
	output : 'eix-functions.sh',
	command : [ echo, '#!' + eprefix_default + '/bin/cat ' + eixdatafile ],
	capture : true,
)
install_data(c, install_dir : bindir, install_mode : 'rwxr-xr-x')

foreach file : zsh_completions
	file_sed = file + '.subst'
	path_in = files(join_paths('zsh', file + '.in'))
	configure_file(
		input : path_in,
		output : file_sed,
		command : [ sed, sed_args, path_in ],
		capture : true,
	)
	path_sed = join_paths(builddir, file_sed)
	c = configure_file(
		input : path_sed,
		output : file,
		configuration : subst,
	)
	install_data(c, install_dir : zsh_completion_path)
endforeach

if extra_doc
	foreach p : [
		[ 'eix-db.txt', docdir ],
		[ 'eix-xml.txt', docdir ],
		[ 'eix-xml.xsd', htmldir ],
	]
		file = p.get(0)
		c = configure_file(
			input : files(join_paths('doc', file + '.in')),
			output : file,
			configuration : subst,
		)
		install_data(c, install_dir : p.get(1))
	endforeach
	docinput = join_paths(sourcedir, 'doc')
	rst2html = files(join_paths('doc', 'call_rst2html.sh'))
	foreach file : [
		'eix-db',
		'eix-xml',
	]
		txtfile = join_paths(builddir, file + '.txt')
		htmlfile = file + '.html'
		htmldest = join_paths(builddir, htmlfile)
		fallback = join_paths(sourcedir, 'doc', htmlfile)
		r = run_command(sh, rst2html,
			txtfile, htmldest, fallback, docinput, dev_null)
		if r.returncode() != 0
			error('call_rst2html.sh failed for ' + htmlfile)
		endif
		install_data(htmldest, install_dir : htmldir)
	endforeach
endif

sed_manpage = '''test -d "$3" || mkdir -- "$3" && sed \
-e 's/$Date:[^,]*, *\([^)]*\)[^\$]*\$/\1/' \
-e 's,\([BI]\)<\([^>]\+\)>,\\f\1\2\\fR,g' \
-e 's/\xc3\x84/\\(:A/g' \
-e 's/\xc3\x96/\\(:O/g' \
-e 's/\xc3\x9c/\\(:U/g' \
-e 's/\xc3\xa4/\\(:a/g' \
-e 's/\xc3\xb6/\\(:o/g' \
-e 's/\xc3\xbc/\\(:u/g' \
-e 's/\xc3\x9f/\\(ss/g' \
-- "$1" >|"$2"
'''
foreach l : manpages
	file = l + '-eix.1'
	file_subst = file + '.subst'
	configure_file(
		input : files(join_paths('manpage', file + '.in')),
		output : file_subst,
		configuration : subst,
	)
	manpagedir = join_paths(builddir, l)
	manpagefile = join_paths(manpagedir, 'eix.1')
	r = run_command(sh, '-c', sed_manpage, 'sh',
		join_paths(builddir, file_subst),
		manpagefile,
		manpagedir)
	if r.returncode() != 0
		error('sed failed: ' + file)
	endif
	mansubdir = mandir
	if l != 'en'
		mansubdir = join_paths(mandir, l)
	endif
	mansubdir = join_paths(mansubdir, 'man1')
	install_data(manpagefile, install_dir : mansubdir)
endforeach

ln_s = '''test -d "$DESTDIR$3" || mkdir -p -- "$DESTDIR$3"
ln -s -- "$1" "$DESTDIR$2" || \
ln -- "$1" "$DESTDIR$2" || \
cp -p -- "$1" "$DESTDIR$2"
'''
foreach l : inst_link
	meson.add_install_script(sh.path(), '-c', ln_s, 'sh',
		l.get(0), join_paths(bindir, l.get(1)), bindir)
endforeach

install_in = [
	[ join_paths('src', 'eixrc'), '00-eixrc', join_paths(sysconfdir, 'eixrc') ],
]
foreach p : install_in
	file = p.get(1)
	c = configure_file(
		input : files(join_paths(p.get(0), file + '.in')),
		output : file,
		configuration : subst,
	)
	install_data(c, install_dir : p.get(2))
endforeach

message('\n-D'.join(result) + '\n\n' +
	'Added CXXFLAGS:\n' + ' '.join(cxxflags_add) + '\n\n' +
	'Added LDFLAGS:\n' + ' '.join(ldflags_add) + '\n')

# TODO:
# Insert flags _in front_ of user-specified CXXFLAGS/LDFLAGS.
# Do not add flags if they are already in CXXFLAGS/LDFLAGS.
