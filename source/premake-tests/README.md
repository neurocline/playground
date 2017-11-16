# Premake style guide

Use the `file=FILE` option when generating one or more of these
samples, e.g.

```
premake5 --file=test_1.lua vs2017
```

We will focus on Visual Studio 2017 first, then add other platforms.

## Simplest Premake script

Premake has a lot of defaults. This is close to the minimal Premake script, in that it
demonstrates multiple targets.

```
workspace "test1"
    configurations { "Debug", "Release" }
    platforms { "x32", "x64" }

project "test1"
    kind "ConsoleApp"
    language "C++"
    files { "**.cpp", "**.h" }
```

This has numerous defects, but will produce a workable VS2017 project with C++ code in it.
We assume there is a trivial `main.cpp` with a valid `main` function in it.

```
premake5 --file=test1.lua vs2017
```

This does reveal some Premake warts. At the user level, our solution platforms are named "Win32"
and "x64", even though we claimed they were labelled "x32" and "x64". This is due to legacy code
in the VC generator, I'm guessing. However, if we mess with this, we get weirdness in the generated
project files.

This puts solution and project files in the current directory.

The output directory is `bin/PLATFORM/CONFIGURATION`, and the intermediate directory is
`obj/PLATFORM/CONFIGURATION`. The character set is Unicode. Warning level is `/W3`. Incremental
linking is enabled. Everything else is Visual Studio defaults.

## Put output in build folder

We'd like our generated objects to all go in a single directory, so we can avoid having many
gitignore rules. Our repo has a top-level `build/` directory that we want to put artifacts into.

```
local ROOT = "../../" -- we are two levels from the top

workspace "test2"
    location(ROOT .. "build/test2")
    configurations { "Debug", "Release" }
    platforms { "x32", "x64" }

project "test2"
    kind "ConsoleApp"
    language "C++"
    files { "**.cpp", "**.h" }
```

When we generate this, we should see our project files in the `build` directory. Note that code generation
will put files relative to our project files by default; this can often be good enough.

Premake context variables can be accessed as strings, so that for example we could name directories and
files with configuration or platform tags. Maybe we generate multiple platforms on the same machine
(Xcode and Gmake, for example). These are called value tokens in the Premake documentation, and are
slots in context-specific objects. The `wks` object is available inside a `workspace` section (and any
nested `project` sections), the `prj` object is available inside a `project` section. There are also
global context objects `cfg` and `file`.

This is done with string interpolation variables - note that these only work inside Premake commands,
sadly, so we can't use Lua temporaries to remove duplication.

```
local ROOT = "../../" -- we are two levels from the top
local PROJECT = ROOT .. "build/test3"

workspace "test3"
    configurations { "Debug", "Release" }
    platforms { "x32", "x64" }
    location(PROJECT)

project "test3"
    kind "ConsoleApp"
    language "C++"
    files { "**.cpp", "**.h" }
    objdir(PROJECT .. "/bin_%{cfg.platform}_%{cfg.buildcfg}/obj")
    targetdir(PROJECT .. "/bin_%{cfg.platform}_%{cfg.buildcfg}")
```

Now, output will be in `bin_x64_Debug` and so on, and the object files will be in an `obj`
directory inside the relevant `bin` directory.

In general, build output should be put in unique directories, to minimize the chance of build
errors for build systems that use timestamps instead of something more unique. This is why we
don't have just a single build output directory.

## Static libraries

We won't just have a single project containing all our source. In a bigger program, we'll want to
have multiple project files, each containing a logical bit. This is different from the case where
we are using libraries; libraries will be brought in through a vendor mechanism, even for our own
libraries.

In this case, we'll introduce a second source file for our library.

```
local BUILD = "../../build/test4" -- we are two levels from the top

workspace "test4"
    configurations { "Debug", "Release" }
    platforms { "x32", "x64" }
    location(BUILD)

project "cmd"
    kind "ConsoleApp"
    language "C++"
    files { "main2.cpp"}
    links { "lib4" }

project "compute"
    kind "StaticLib"
    language "C++"
    files { "compute.cpp"}
```

The type of a static library in Premake is `StaticLib`. The `links` command adds a link reference
to our main project for the library project.

If you build and look at the output, since we removed the `objdir` and `targetdir` commands, you
will see the following tree:

```
test4/
  bin/
    x32/
      Debug/
        cmd.exe
        cmd.ilk
        cmd.pdb
        compute.lib
        compute.pdb
  obj/
    x32/
      Debug/
        cmd/
          cmd.tlog
          cmd.log
          main.obj
          vc141.pdb
        compute/
          compute.tlog
          compute.log
          compute.obj
          compute.pdb
  cmd.vcxproj
  compute.vcxproj
  test4.sln
```

Premake by default puts all binaries and objects in unique locations, to avoid object files of the
same name but in different projects clobbering each other. If we add `objdir` and `targetdir`
commands, we'll need to make sure to provide unique locations for each, which we can do by adding
the `prj.name` variable to our computed string.

With this pattern, we can build up an arbitrarily complicated project, while still partitioning
things into logical groups.

## Generating for XCode

We can generate XCode projects by changing the action.

```
premake5 --file=test4.lua xcode4
```

This will generate XCode projects on any system, not just on a Mac. This is sometimes convenient for
doing dry-run testing or in generating projects that you can check in to source control.

Xcode projects use bunde files, which show up as directories with files in them - we get
a `*.xcworkspace` for the top level, and `*.xcodeproj` for each project in the workspace.

## Local variables and Premake

We can clean up the code and remove duplication by using a few Lua variables.

```
local ROOT = "../../build/" -- we are two levels from the top
local PROJECT = "test3c"
local WORKSPACE = ROOT .. PROJECT
local BIN = WORKSPACE .. "/bin_%{cfg.platform}_%{cfg.buildcfg}"

workspace(PROJECT)
    configurations { "Debug", "Release" }
    platforms { "x32", "x64" }
    location(WORKSPACE)

project(PROJECT)
    kind "ConsoleApp"
    language "C++"
    files { "main.cpp" }
    objdir(BIN .. "/obj")
    targetdir(BIN)
```

The Premake code introduces a large number of variables into the scope of our Lua code, so we
name ours uppercase to avoid collisions and to make it clear that these are our variables and
not from the system or Premake. This is a convention, not any kind of requirement, of course.

It's important to keep the number of variables low, and also to note that local variables stay
in the scope of the block they were created in. If you wish global variables, omit the `local`
token from the declaration (and you may want to make global variable names more verbose).

## C++ language

Premake has a relatively new keyword called `cppdialect` that can be used to select a specific version
of C++. As of Premake 5.0.0-alpha12, this is the behavior

| Premake | Visual Studio | GCC/Clang |
| ------- | ------------- | --------- |
| nothing | nothing | nothing |
| `cppdialect "C++11"` | nothing | `-std=c++11` |
| `cppdialect "C++14"` | `/std:c++14` | `-std=c++14` |
| `cppdialect "C++17"` | `/std:c++latest` | `-std=c++17` |

Visual Studio didn't add C++ version selection until C++14; its history of compliance to that point
was weak. I expect Premake emits `/std:c++latest` instead of `/std:c++14` because the latter didn't
exist until VS2017 version 15.3, which was August 2017, after this version of Premake was baked.

In Visual Studio 2015, C++14 is the default. VS2015 was the first version with mostly-complete C++11
support, and did not have full C++14 support. See [Visual C++ Language Conformance](https://docs.microsoft.com/en-us/cpp/visual-cpp-language-conformance) for details on C++ standard compliance in Visual C++. Apparently, Visual Studio 2015 version 15.5 supports about 75% of C++17 features.
