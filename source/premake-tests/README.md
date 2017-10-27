# Premake style guide

Use the `file=FILE` option when generating one or more of these
samples, e.g.

```
premake5 --file=test_1.lua vs2017
```

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
