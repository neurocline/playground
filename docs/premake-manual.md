Premake Manual
==============

This is tidbits towards a complete Premake 5 manual.

Introductory Notes
==================

Here are the three most important things for you to keep in mind.

1. Premake is not a build system, it generates makefiles for build systems.
2. Premake scripts are Lua code and executed as Lua programs.
3. Your Premake scripts are declarative code, not procedural code.

Premake is a standalone executable that does source-to-source-transitions in a self-contained
fashion. This means that you can actually generate makefiles for any platform and any
system on a single system; you could generate Xcode projects from a Windows machine, for
example, or Visual Studio projects from a Linux machine. Since part of the reason for Premake
is as a cross-platform build system generator, this isn't just academic, but lets a single
user generate projects that will probably work, without needing multiple machines to test on.
This also makes it easier to test out your Premake scripts, since you can investigate the
output by hand.

Premake is a power-user tool. It will do its best to tell you about syntactic errors in
project scripts, but most semantic errors will simply result in nothing being generated
for that section. Filters and files sections, when they fail to match anything, will just
assume that this was intended. There's currently no way to semantically debug a Premake
script other than removing parts of it; one useful feature would be a verbose log that
you could enable to troubleshoot problems.

In lieu of that, we'll present some ways that you can narrow in on problems in your
Premake scripts.

Project structure
=================

Premake expects Lua scripts of a specific form to be presented to the `premake` tool.
The default name for the top-level script is `premake5.lua` in the current working
directory; alternatively, you can pass the path to the top-level script on the premake
command-line with `-file <path>`.

All your Premake scripts combined are executed as a single function with the Lua call
`dofile(<user_script>)`. Premake itself is partially Lua code that sets up a default
environment, calls your Premake script, then uses the fully-specified environment to
generate makefiles.

Premake itself is customizable to an almost arbitrary degree; since Premake operations
are defined by Lua code, and you can override or introduce new behavior as you desire,
you could even ignore existing Premake behavior in favor of a new system. Just like with
all open-ended systems, it's best to stick to existing metaphors and not replace them,
or even to extend them more than absolutely necessary.

Premake has two existing makefile constructs: the `workspace`, and the `project`. Each
workspace is the parent for some number of projects. You can have multiple workspaces in
a single invocation of Premake; this is atypical, but some workflows may find the need
for multiple workspaces generated all at once.

At generation time, workspaces and projects turn into build system artifacts. In the major
build systems, these break down as follows:

| Premake | Visual Studio | Xcode | make |
| ------- | ------------- | ----- | ---- |
| `workspace` | `*.sln` | `*.xcworkspace` | top-level `Makefile` |
| `project` | `*.vcxproj` | `*.xcproj` | project `Makefile` |

There are third-party generators for CMake, Ninja, and several other build systems.

Include Paths
=============

Many languages have some kind of source include command - for example, C/C++ has
`#include`, Go has `import`. There is also further differentiation in some build
environments between "user includes" and "system includes".

`includedirs { paths... }` adds `paths...` to the set of include directories that
is used for both user and system includes.

`userincludedirs { paths... }` adds `paths...` to the set of user-only include
directories - if the targetted build system does not have the concept of user-only
includes, then this has the same effect as `includedirs`.

`sysincludedirs { paths...}` adds `paths...` to the set of system include directories.
Note that this is NOT "system-only includes", since some languages and their build
systems search both user paths and system paths for user includes. However, some
build systems treat "system includes" as semi-privileged entities, for example disabling
warnings. If the targetted build system does not have the concept of system includes,
then this has the same effect as `includedirs`.

Premake command line
====================

The following examples assume that the `premake5` binary is somewhere on the `PATH`.

The general form of the command-line is

`premake5 [<option>...] <action> [<argument>...]`

A typical command might look like this:

```
premake5 vs2017
```

which will look for a file in the current directory named `premake5.lua` and then read it
and use its instructions to generate Visual Studio 2017 project files (.sln and .vcxproj).

Get simple help (mostly to remind you what the actions are) with `premake5 --help`.

Actions
-------

Premake5 commands expect a single verb called `action` - this is the action to take,
typically to generate project files for a specific build system. For a full list, see
**filter:actions** in the command help below. An abbreviated list is as follows:

```
gmake
vs2017
xcode4
```

Options
-------

Premake has a number of built-in options, and more can be added by your Premake5 script. The most
common are

- `--fatal`: treat warnings in scripts as errors
- `--file=<script>`: read `<file>` as the top-level script (defaults to `premake5.lua`)
- `--insecure`: skips SSH cert checks for package downloads (needed in some firewalled environments)

Since Premake5 is a standalone program and not dependent on any operating system libraries, you can
generate project files for any target from any machine. By default, it assumes the target os is
the same as the current os. This is mostly relevant to the `gmake` actions.

- `--os=<value>`: select OS, one of `aix`, `bsd`, `haiku`, `hurd`, `linux`, `macosx`, `solaris`, `windows`

You can select a specific compiler (relevant to the `gmake` actions only)

- `--cc=<value>`: select a C/C++ compiler, one of `clang` or `gcc`.
- `--dc=<value>`: select a D compiler, one of `dmd`, `gdc`, or `ldc`.
- `--dotet=<value>`: select a .NET compiler, one of `msnet`, `mono`, or `pnet`

Premake is itself a large set of Lua scripts. You can run with a modified version of these scripts
by pointing Premake to a new source for the scripts; this lets you fix or update behavior in
Premake itself without waiting for a new release (e.g. supporting the latest version of Visual
Studio). This is definitely an advanced feature, and largely only used by Premake developers.

- `--scripts=<path>`: add `<path>` to the set of paths that Premake searches for scripts
- `--systemscript=<file>`: override default system script (`premake5-system.lua`)

It is possible to have Premake stop and wait for you to enter extra information specific to your
build. See the **interactive** section for more details.

- `--interactive`: opens an interactive command prompt.

There are also some help options:

- `--help`: display command-line help
- `--version`: show Premake version

Interactive
-----------

For debugging, you can cause a REPL prompt to come into existence. There are two ways.

On the command-line, if you add `--interactive`, a REPL prompt will be started after project
scripts have been processed and Premake data set up. You'll need knowledge of Premake internals,
but the REPL will let you enter and execute arbitrary Lua commands to your heart's content.

In code, you can call `debug.prompt()`, which will start up the REPL prompt at that point
in the script parsing. This may be useful to narrow down what you are looking at - perhaps
some later part of your project scripts is overwriting data that you wouldn't see if you opened
the REPL at the end of parsing.

You'll need to know Lua and something about the internals of Premake. The source code to Premake
is available, which helps, and the StackOverflow and Premake mailing list both have information
on them.

For example, this will print the top-level of the `premake` table, which holds most Premake
information:

```
for k,v in pairs(premake) do print(k,v) end
```

For real debugging, I assume you would add some debugging code to your Premake scripts that you
can call as helper functions. It would be nice if this was actually part of Premake itself, or
added when the REPL loop is called.

There doesn't appear to be any way to exit the REPL and cause processing to continue.

Premake commands
================

architecture
------------

The Premake `architecture` command lets you specify the system architecture targeted by
the configuration. This is necessary for non-trivial configurations. The following
architectures are built-in, but more could be added by add-on modules.

- `architecture "x86"` sets the 32-bit x86 architecture
- `architecture "x68-64"` sets the 64-bit x86-64 architecture
- `architecture "ARM"` sets the ARM architecture

Remembering that Premake has both `configurations` and `platforms` and both combine to
make a configuration. Premake claims that platform names have no predefined meaning, but
this is not precisely true. If no platform names are supplied, then Premake uses whatever
the toolset defaults are.

- toolset `msc`: defaults are "Win32", "Win64"

This is what Premake appears to do by default for the `msc` toolset. For any other platform
name, it just assumes `Win32` which means x86 architecture, and that's probably not what you want.

- `platforms { "Win32" }` == `architecture "x86"`
- `platforms { "x64" }` == `architecture "x86_64"`
- `platforms { "x32" }` == `architecture "x86"` (Visual Studio sees this as `Win32` platform)

Note that `Win64` is not a name recognized by Premake's `msc` toolset, and if you use it, you
will get a configuration name that is "Debug Win64". If the platform name matches the architecture,
Premake can suppress it from the generated project. This is all due to the fact that Premake's
system for identifying the parts of a build don't precisely map to that used by Visual Studio,
or Xcode.

If your platform names are anything else, you will need to add `architecture`
commands in filters to set up configurations properly. The pattern looks like this

```
workspace "test"
    configurations { "Debug", "Release" }
    platforms { "Win32Fast", "Win64Slow" }

project "test"
    kind "ConsoleApp"
    language "C++"
    files { "**.cpp", "**.h" }
    
    filter "platforms:Win32*"
        architecture "x86"
    filter "platforms:Win64*"
        architecture "x86_64"
```

You could even do crazy things like make Debug-Win32Fast be x64 architecture. I assume that's a bad
idea, but Premake wouldn't care.

Or, you may wish to use the `platforms` list for another axis, like console app versus static
library versus dynamic library versus gui app, and just build with a single architecture. Or have
your architecture encoded into your configuration name. These are all just strings, and the only
thing Premake does is check platform names against toolsets to look for defaults to set.

This calls for a best practices document.

[Premake5 wiki: architecture](https://github.com/premake/premake-core/wiki/architecture)

buildoptions
------------

The Premake `buildoptions` command adds the specified strings to the compiler build options. As such, this
is very platform-specific, and needs to be wrapped in a `filter` to target the correct build
platform. This is an escape valve to offer access to compiler features that Premake is unaware of.

Options are supplied to the compiler (typically a command-line) in the order that they appear in the
Premake script.

Use `filter` to limit the application of buildoptions to relevant environments. The options to
best filter on are `action` and `toolset`. For Visual Studio and Xcode, it's generally arbitrary
as to which to use. I prefer `toolset` because it's possible to conceive of an action that generates
code using multiple toolsets.

The following lists are incomplete, and meant more as examples on how to apply compiler settings for
Visual Studio, GCC, Clang etc.

```
    filter { "toolset:msc*" }
        buildoptions { '/permissive-', '/diagnostics:caret' }
    filter { "toolset:clang*" }
        buildoptions { '-std=c++17' }
```

The following is a list of buildoptions that have been turned into higher-level Premake commands
over time. Prefer the cross-platform high-level Premake commands over compiler-specific buildoptions.

| Visual Studio | GCC | Clang | Premake |
| ------------- | --- | ----- | ------- |
| n/a | `-std=c++11` | `-std=c++11` | `cppdialect "C++11"` |
| `buildoptions "/std:c++14"` | `-std=c++14` | `-std=c++14` | `cppdialect "C++14"` |
| `buildoptions "/std:c++latest"` | `-std=c++17` | `-std=c++17` | `cppdialect "C++17"` |

[Premake5 wiki: buildoptions](https://github.com/premake/premake-core/wiki/buildoptions)

cppdialect
----------

This used in conjunction with `language "C++"` to further specify the kind of C++ language
desired.

```
language "C++"
cppdialect "C++14"
```

Ideally, this would just be part of `language`.

[Premake5 wiki: buildoptions](https://github.com/premake/premake-core/wiki/buildoptions)

defines
-------

The Premake `defines` command adds preprocessor or compiler symbols to a project. These
are added in the order specified in the script, and may have values.

Note that it's usual to have defines restricted to a subset of the build targets. Use `filter`
to wrap `defines` lines to only apply to specific toolsets or configurations.

```
    filter { "configurations:debug" }
        defines { "_DEBUG" }
    filter { "toolset:msc*" }
        defines { "_ITERATOR_DEBUG_LEVEL=0", "_CRT_SECURE_NO_WARNINGS", "_SCL_SECURE_NO_WARNINGS" }
```

[Premake5 wiki: defines](https://github.com/premake/premake-core/wiki/defines)

filter
------

Premake is a declarative system, even though it looks like a procedural system. When you
declare a filter, it stays in existence until another filter is declared, or a new project
or workspace is entered. A filter is used to restrict following declarations to a subset of
the Premake permutations.

Only one filter can be in operation at a time. There is a single filter that you change the
value of; these aren't nested or localized in any way, although you get reasonable behavior
due to the fact that the filter is reset when you enter a scope.

The null filter is `filter {}`, which means "filter nothing"; the null filter is active
at the entry to a workspace or project section.

Note that the filter syntax isn't a clean syntax - it evolved a bit at a time, and is not
a general-case propositional calculus. Some fiddling may

A filter is a list of zero or more items; zero items is the null filter. Each item is a
string describing a pattern. A list of items has an implicit `and` between each item; the
filter `filter {"configurations:Debug", "toolset:msvc*"}` is read as "the subset that is
the `Debug` configuration and any toolset starting with `msvc`".

Each item has a prefix and a pattern, separated by `:`. In `filter {"configurations:Debug"}`,
`configurations` is the prefix. The prefix selects a Premake field (think object or variable),\
which is one of:

- `action`
- `architecture`
- `configurations`
- `files`
- `kind`
- `language`
- `options`
- `platforms`
- `system`
- `toolset`

The pattern selects one or more values of the field. The pattern can be a full text-match,
a wildcard match using `*` or `**` wildcards (the latter relevant for the `files` field),
and it can contain some boolean operators.

- `filter {"configurations:Debug or Release"}` means "the Debug or Release configuration"
- `filter {"configurations:not Debug"}` means "any configuration except the Debug configuration"
- `filter {"toolset:msvc*"}` means "any toolset starting with the characters 'msvc'"

Pattern is slightly complicated by the fact that you can add a field to a sub-pattern.

- `filter "system:windows or language:C#"` means "the Windows system or the C# language"

There is old behavior that lets the prefix be optional - if the prefix is omitted, this means
the `configurations` field. Note specifically that the field omitted inside a pattern does NOT
mean `configurations` field, it means "use the most recently specified field". This probably
means that the filter parsing grammar starts out with the default field being `configurations`
and sets it each time a field is specified.

### action

These are the predefined actions. Typically, you'll be filtering on `action:vs*`, `action:xcode*`,
or `action:gmake*`, and the last one needs some care taken with it, because the compiler and
project file for Visual Studio and Xcode are more tightly tied together.

- `clean` - remove all binaries and generated files
- `codelite` - Generate CodeLite project files
- `gmake` - Generate GNU makefiles for POSIX, MinGW, and Cygwin
- `gmake2` - Generate GNU makefiles for POSIX, MinGW, and Cygwin
- `vs2005`
- `vs2008`
- `vs2010`
- `vs2012`
- `vs2013`
- `vs2015`
- `vs2017` - Generate Visual Studio 2017 project files
- `xcode4` - Generate Xcode 4 project files

The `action` is global to the entire Premake invocation, comes from the command-line, and
is a string stored in the `_ACTION` global variable.

Note that since actions can be added via addons, it is not possible to list every possible
action. It would be nice if `premake --verbose` would show actions as defined in the scope
of the Premake script being processed.

Note that it's typical for `clean` to be overriden by a Premake script, since Premake itself
doesn't track what it wrote in a previous operation, nor can it know about files created by
build systems themselves.

### architecture

### options

The `options` field gives you access to command-line options. These are stored as a table
in the `_OPTIONS` global variable. As such, there are no default values for options; these
are defined by your script with the `newoption` function.

[Premake5 wiki: Command Line Arguments](https://github.com/premake/premake-core/wiki/Command-Line-Arguments)

flags
-----

[Premake5 wiki: toolset](https://github.com/premake/premake-core/wiki/flags)

platforms
---------

The `platforms` command declares the set of platforms in your workspace or project. Project
platforms are inherited from the enclosing workspace. Each toolset has a set of default
platforms that are used if no user-specified platforms are offered.

Some platforms have magic behavior. On the `msc` toolset, the following platform
names are recognized and have default behavior.

- `toolset:msc, platform:Win32`: architecture=`x86`
- `toolset:msc, platform:x86`: architecture=`x86`, platform renamed to `Win32` in generated project
- `toolset:msc, platform:x64`: architecture=`x68-64`,

Note that `Win64` is not a name recognized by Premake's `msc` toolset, and if you use it, you
will get a configuration name that is "Debug Win64". If the platform name matches the architecture,
Premake can suppress it from the generated project. This is all due to the fact that Premake's
system for identifying the parts of a build don't precisely map to that used by Visual Studio,
or Xcode.

TBD - move duplicated text out of `architecture` and link it to here, or write up some higher-level
text describing Premake's `configurations` and `platforms` and how they map onto build systems
like Visual Studio and Xcode.

system
------

[Premake5 wiki: toolset](https://github.com/premake/premake-core/wiki/flags)

toolset
-------

Use the `toolset` command to select the specific compiler, linker, etc used to build a project
or configuration. If this is not used, Premake uses a system or IDE default. For example, if
you specific the action to be `vs2017`, then the toolset defaults to `msc-v141`.

Premake knows about the following toolsets out of the box:

- `clang` - Clang
- `dotnet` - C#
- `gcc` - GCC
- `msc` - Microsoft C/C++

You can add your own toolset with a third-party module.

A toolset may have a toolset version, e.g. `msc-v141`. Currently Premake only knows about
versions for Visual Studio.

In general, you'll find that you rely on Premake defaults for toolset. There are edge cases
where we do want to specify toolset.

For example, `premake5 vs2017` where `premake5.lua` contains `toolset "msc-v154"` will create
a Visual Studio 2017 solution and project that uses the older v140 (VS2015) compiler toolchain
for building. It's useful to use the latest IDE even when building with older toolsets.

[Premake5 wiki: toolset](https://github.com/premake/premake-core/wiki/toolset)

### Visual Studio toolset versions

| toolset | name |
| ------- | ---- |
| `v141`  | Visual Studio 2017 |
| `v141_xp` | Visual Studio 2017 - Windows XP |
| `v141_clang_c2` | Visual Studio 2017 - Clang with Microsoft CodeGen |
| `v140` | Visual Studio 2015 |
| `v140_xp` | Visual Studio 2015 - Windows XP |
| `v140_clang_c2` | Visual Studio 2015 - Clang with Microsoft CodeGen |
| `v120` | Visual Studio 2013 |
| `v120_xp` | Visual Studio 2013 - Windows XP |
| `v110 ` | Visual Studio 2012 |
| `v100` | Visual Studio 2010 |

You can specify a Visual Studio toolset with the full string `toolset "msc-v141"` or
with the shortcut `toolset "v141"` - the latter is for backwards compatibility with older
Premake scripts.

Note: there are apparently other toolsets like `v140_clang_3_7` or `LLVM-vs2014` that might exist.

Reference
=========

[premake/premake-core (source code)](https://github.com/premake/premake-core)

[Premake 5 User Guide](https://github.com/premake/premake-core/wiki)

[Premake Development](https://groups.google.com/forum/#!forum/premake-development)

[Premake for Beginners](https://github.com/JohannesMP/Premake-for-Beginners/blob/master/Build/premake5.lua_commented.lua)

[What's new in 5.0](https://github.com/premake/premake-core/wiki/What%27s-New-in-5.0)

Premake expects you to know some Lua.

- [The Lua Programming Language](https://www.lua.org/)
- [Lua 5.3 Reference Manual](https://www.lua.org/manual/5.3/)
- [Programming in Lua (5.0)](https://www.lua.org/pil/contents.html)
- [How to dump a table to console?](https://stackoverflow.com/questions/9168058/how-to-dump-a-table-to-console)

[Microsoft/vswhere](https://github.com/Microsoft/vswhere)

[How to create Conan packages with Premake?](https://github.com/conan-io/conan/issues/1559)

Other build system generators

- [TurkeyMan/premake-cmake](https://github.com/TurkeyMan/premake-cmake) for Premake4, doesn't work in Premake5
- [Geequlim/premake-modules](https://github.com/Geequlim/premake-modules)
- [jimon/premake-ninja](https://github.com/jimon/premake-ninja)
- [Ninja support #141](https://github.com/premake/premake-core/issues/141)
