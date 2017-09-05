#!/usr/bin/env sh

# Check whether the compiler works with static archives and flto.
# This can be used to check for https://github.com/vaeth/mv-overlay/issues/2

set -u

Echo() {
	printf '%s\n' "$*"
}

Die() {
	Echo "
Fatal error: $*" >&2
	exitstatus=1
	exit $exitstatus
}

in_testdir=false
exitstatus=0
RemoveTestdir() {
	trap : EXIT HUP INT TERM
	! $in_testdir || cd .. >/dev/null 2>&1 || {
		trap - EXIT HUP INT TERM
		Die "cannot change to testdir/.."
	}
	in_testdir=false
	! test -d testdir || rm -rf testdir
	trap - EXIT HUP INT TERM
	exit $exitstatus
}
trap RemoveTestdir EXIT HUP INT TERM
test -d testdir || mkdir testdir || Die 'cannot create testdir'
cd testdir || Die 'cannot change to testdir'
in_testdir=:

Echo 'int dummy(int i);
int dummy(int i) {
	return i;
}' >sub.cc || Die 'failed to create sub.cc'
Echo 'int dummy(int i);
int main() {
	return dummy(0);
}' >main.cc || Die 'failed to create main.cc'
Echo "project('test', 'cpp')
cxx = meson.get_compiler('cpp')
cxx_id = cxx.get_id()
stresstest = '''int main() {
	return 0
}'''

flags_fatal = []
foreach o : [
	'-Werror',
	'-Werror=unknown-warning-option',
	'-Wunknown-warning-option',
	'-Wl,--fatal-warnings',
]
	t = flags_fatal
	t += [ o ]
	if cxx.links(stresstest, args : t)
		flags_fatal += [ o ]
	endif
endforeach

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
	if cxx.links(stresstest, args : t)
		flto_flags += [ o ]
		add_global_arguments(o, language : 'cpp')
		add_global_link_arguments(o, language : 'cpp')
	endif
endforeach
executable('main', 'main.cc',
	link_with : static_library('sub', 'sub.cc'))" >meson.build \
	|| Die 'failed to create meson.build'
meson builddir || Die 'failed to execute meson'

ninja -C builddir || Die 'failed to compile static archives with -flto.

A reason might be that you are missing the linker flto plugin.
To establish the latter, something like
	mkdir -p /usr/*/binutils-bin/lib/bfd-plugins
	cd /usr/*/binutils-bin/lib/bfd-plugins
	ln -sfn /usr/libexec/gcc/*/*/liblto_plugin.so.*.*.* .
might be required where you might have to replace * by your architecture
or gcc-version, respectively.'

RemoveTestdir
