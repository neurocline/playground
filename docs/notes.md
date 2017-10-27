# Premake

- [premake](https://premake.github.io/).
- [Wiki](https://github.com/premake/premake-core/wiki)
- [StackOverflow questions](https://stackoverflow.com/questions/tagged/premake)
- [Google groups](https://groups.google.com/forum/#!forum/premake-development)

Current version is 5.0.0.alpha12, released on August 23 2017.

Premake development is happening, but not at the pace needed to overtake CMake.

## Basic premake script

```
-- premake5.lua
workspace "playground"
    configurations { "Debug", "Release" }
    platforms { "Win32", "Win64" }

project "playground"
    kind "ConsoleApp"
    language "C++"
    files { "source/**.cpp", "source/**.h" }
    
    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
    filter "platforms:Win32"
        system "Windows"
        architecture "x86"
    filter "platforms:Win64"
        system "Windows"
        architecture "x86_64"
```

Note lots of boilerplate.

## Side note on project structure

One project, one repository. The top-level of the project/repository should have as few items in it
as possible; suggestion:

```
docs/
source/
tools/
vendor/
.editorconfig
premake5.lua
README.md
```

If the project doesn't need embedded tools, then `tools` is not present. If there is no vendored
(external) source, then `vendor` is not present.

There is a single top-level doc file. The convention is that this is called `README`. Github supports
several formats, among them, Markdown and ReStructuredText, so we follow this to be friendly to Github
users. The top-level README.md should point to any other documentation.

The `vendor` directory can contain copies of external source, git modules pointing to external source,
or package instructions used to reference external source.

The .editorconfig file is optional but strongly recommended; this is where macro code style rules
are kept. See [Editorconfig](http://editorconfig.org/). Most modern editors support EditorConfig.

## Build into a single place

Put all generated files into a `build` directory; this allows for easy cleaning, and easy rules for
what should and shouldn't be checked in to source control.

## More on Visual Studio

The default C++ wizard creates a solution file with this

```
Microsoft Visual Studio Solution File, Format Version 12.00
# Visual Studio 15
VisualStudioVersion = 15.0.27004.2005
MinimumVisualStudioVersion = 10.0.40219.1
Project("{xxxx}") = "someproj", "someproj\pg2.vcxproj", "{yyyyy}"
```

The `VisualStudioVersion` and `MinimumVisualStudioVersion` are relatively new.

The default solution platform names are `x86` and `x64`. However, the platforms themselves are
still called `Win32` (32-bit Windows) and `x64` (64-bit Windows). Strange, and yet another slight
difference for project-generation tools to account for.

There's also a new `GlobalSection`:

```
    GlobalSection(ExtensibilityGlobals) = postSolution
        SolutionGuid = {73635D00-CC78-4227-B368-BF6954D86C10}
    EndGlobalSection
```

The referenced GUID is not in any generated file (`*.sln` or `*.vcxproj`), so it must be a
new Visual Studio internals GUID. Google says that this is related to TFS; the solution file
will get checked out automatically if you open the solution from TFS. So we probably don't want
this anywhere. And this is not new. I just somehow never saw this before. `ExtensibilityGlobals`
is where Visual Studio add-ins can store solution-wide information. Maybe Microsoft assumes
that everyone should use TFS now?

## Sigh

OK. A clean project with the Visual Studio 2017 wizard.

1. File -> New -> Project
2. Windows Console Application

Interesting. The options are gone, so we get a project with a precompiled header via their
stdafx.h convention. There doesn't appear to be an "empty project" option.

The solution configuration names are `Debug` and `Release`. The solution platform names are `x86` and `x64`.
Of course, inside the solution, the platform names are `Win32` and `x64`, because that's what all the tooling
expects (msbuild, etc).

```
  <ItemGroup Label="ProjectConfigurations">
    <ProjectConfiguration Include="Debug|Win32">
      <Configuration>Debug</Configuration>
      <Platform>Win32</Platform>
    </ProjectConfiguration>
    ...
    <ProjectConfiguration Include="Release|x64">
      <Configuration>Release</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
  </ItemGroup>
```

Premake still likes solution platform names of `Win32` and `x64` - the preferred platform names for
Premake are:

```
    platforms { "x32", "x64" }
```

There is a default rule that maps `platform:x32` to `architecture:Win32` and `platform:x64` to
`architecture:x64`. With any other platform names, you can still set the architecture names, but
you get a weird combination of user name and architecture name for the solution platform name.

E.g. if you have this:

```
-- premake5.lua
workspace "playground"
    configurations { "Debug", "Release" }
    platforms { "Win32", "Win64" }

project "playground"
    kind "ConsoleApp"
    language "C++"
    files { "source/**.cpp", "source/**.h" }
    
    filter "platforms:Win32"
        system "Windows"
        architecture "x86"
    filter "platforms:Win64"
        system "Windows"
        architecture "x86_64"
```

then the user-visible labels are fine, but the actual solution looks like this:

```
    <ProjectConfiguration Include="Debug|Win32">
      <Configuration>Debug</Configuration>
      <Platform>Win32</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Debug|x64">
      <Configuration>Debug</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Debug Win64|Win32">
      <Configuration>Debug Win64</Configuration>
      <Platform>Win32</Platform>
    </ProjectConfiguration>
    <ProjectConfiguration Include="Debug Win64|x64">
      <Configuration>Debug Win64</Configuration>
      <Platform>x64</Platform>
    </ProjectConfiguration>
```

There are extra combinations that are nonsense, and repetitive (e.g. there should be just the Debug|Win32
and Debug|x64 targets). Not all the project file has this spam - some have 1 extra per 2, some have just the
minimum). At the Visual Studio IDE level, everything looks normal, so somehow things are being ignored. And
that is because the solution tells the IDE what to show

```
    GlobalSection(ProjectConfigurationPlatforms) = postSolution
        {9DF067D1-09A6-E0C6-929A-DA9DFE4ED7CA}.Debug|Win32.ActiveCfg = Debug|Win32
        {9DF067D1-09A6-E0C6-929A-DA9DFE4ED7CA}.Debug|Win32.Build.0 = Debug|Win32
        {9DF067D1-09A6-E0C6-929A-DA9DFE4ED7CA}.Debug|Win64.ActiveCfg = Debug Win64|x64
        {9DF067D1-09A6-E0C6-929A-DA9DFE4ED7CA}.Debug|Win64.Build.0 = Debug Win64|x64
        {9DF067D1-09A6-E0C6-929A-DA9DFE4ED7CA}.Release|Win32.ActiveCfg = Release|Win32
        {9DF067D1-09A6-E0C6-929A-DA9DFE4ED7CA}.Release|Win32.Build.0 = Release|Win32
        {9DF067D1-09A6-E0C6-929A-DA9DFE4ED7CA}.Release|Win64.ActiveCfg = Release Win64|x64
        {9DF067D1-09A6-E0C6-929A-DA9DFE4ED7CA}.Release|Win64.Build.0 = Release Win64|x64
    EndGlobalSection
```

The extra text (e.g. "Debug Win64|x64") is how, I think, Visual Studio remembers the user platform
string. The label at this level (what the user and msbuild see) maps to the corresponding entry
in the vcxproj file.

This has to be a bug in Premake5.
