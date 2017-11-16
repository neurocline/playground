Premake Manual
==============

This is tidbits towards a complete Premake manual.

Introductory Notes
==================

Premake is a power-user tool. It will do its best to tell you about syntactic errors in
project scripts, but most semantic errors will simply result in nothing being generated
for that section. Filters and files sections, when they fail to match anything, will just
assume that this was intended. There's currently no way to semantically debug a Premake
script other than removing parts of it; one useful feature would be a verbose log that
you could enable to troubleshoot problems.

In lieu of that, we'll present some ways that you can narrow in on problems in your
Premake scripts.

Premake commands
================

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
or workspace is entered. At the entry to a workspace or project, the null filter `filter {}` is active.

You can filter on identifiers from one or more fields, which are:

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

flags
-----

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

[Premake 5 User Guide](https://github.com/premake/premake-core/wiki)

[Premake Development](https://groups.google.com/forum/#!forum/premake-development)

[Premake for Beginners](https://github.com/JohannesMP/Premake-for-Beginners/blob/master/Build/premake5.lua_commented.lua)
