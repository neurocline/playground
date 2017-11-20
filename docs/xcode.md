Xcode notes
===========

Search paths
------------

Include files are searched for in this order:

1. For `#include "path"` only, search the directory of the current file.
2. For `#include <path>"` only, search the directories in `USER_HEADER_SEARCH_PATHS` (`-iquote` on the command line), in the order specified.
3. For any `include`, search the directories in `HEADER_SEARCH_PATHS` (`-I` on the command line) in the order specified.
4. For any `include`, search the directories in `SYSTEM_HEADER_SEARCH_PATHS` (`-isystem` on the command line) in the order specified.
5. For any `include`, standard system directories as baked into the compiler are searched.
6. For any `include`, search the directories specified with `-idirafter` in the order specified.

If `ALWAYS_SEARCH_USER_PATHS` is set, then change step 2 to "for any `include`", and not just quote-includes.
Note that this option is deprecated and should not be used any more.

Note that for GCC at least (and probably Clang), `-isystem` "marks" the directory as a system directory, so
that it gets the same treatment as other system directories: these have warnings disabled (except for
explicit `#warning` directives), and cannot be overriden in the search order by "normal" directories included
with `-I` or `-iquote`.

### Old stuff (delete me)

As of Xcode 8.3 we have a bit of a mess. Apple is trying to clean things up but be backwards-compatible
at the same time, and this makes project generators behave relatively poorly.

There are three search paths in Xcode vs two in Clang/GCC.

- `i<path>` in compiler, `#include "header"` in code, is stored in `USER_HEADER_SEARCH_PATHS`
- `I<path>` in compiler, `#include <header>` in code, is now stored in `SYSTEM_HEADER_SEARCH_PATHS` but was stored in `HEADER_SEARCH_PATHS` before 8.3

Some compilers do not support separate system-level paths and user-level paths. The standard doesn't require
this distinction, it just allows it.

Actually, I think Xcode may actually have broken the standard now. I believe the standard says that `#include "path"`
means "search in system locations first, then if not found search in user locations", and that `#include <path>`
says "search in system locations only".

Actually, I don't know how this is meant to work. The documentation is not precise in some places. Here are the clear
parts:

- `USER_HEADER_SEARCH_PATHS` is for headers in quotes, e.g. `#include "header"`.
- if `ALWAYS_SEARCH_USER_PATHS` is set to true, then `#include <header>` uses `USER_HEADER_SEARCH_PATHS` before `HEADER_SEARCH_PATHS`. If set to false, then `include "header"` uses `USER_HEADER_SEARCH_PATHS` only.

Hmm, let's rewind a bit. Clang and GCC build up header search paths with these options:

- `-I<dir>`
- `-i<dir>`
- `-iquote <dir>` Add directory to QUOTE include search path
- `-isystem <dir>` Add directory to SYSTEM include search path
- `-isystem-after <dir>` Add directory to end of the SYSTEM include search path

There are some more obscure ones

- `-idirafter <dir>` Add directory to AFTER include search path
- `-iframework <dir>` Add directory to SYSTEM framework search path
- `-iframeworkwithsysroot <directory>` Add directory to SYSTEM framework search path, absolute paths are relative to -isysroot
- `-isysroot <dir>` Set the system root directory (usually /)

I think that `-iquote` is an alias of `-i` and that `-isystem` is an alias of `-I`. Actually, it looks like `-i` is gone
from the world, or it's just not documented any more.

GCC has this tidbit: "Directories specified with -iquote apply only to the quote form of the directive, #include "file". Directories specified with -I, -isystem, or -idirafter apply to lookup for both the #include "file" and #include <file> directives."
This seems backwards to what the C/C++ standards say. However, it's what GCC does for search order that's important, and
I'm pretty sure Clang is copying it.

The lookup order is as follows:

1. For the quote form of the include directive, the directory of the current file is searched first.
2. For the quote form of the include directive, the directories specified by -iquote options are searched in left-to-right order, as they appear on the command line.
3. Directories specified with -I options are scanned in left-to-right order.
4. Directories specified with -isystem options are scanned in left-to-right order.
5. Standard system directories are scanned.
6. Directories specified with -idirafter options are scanned in left-to-right order.

This is still worded vaguely. This is how I think it works.

- Both quote and angle use `HEADER_SEARCH_PATHS`.
- Only quote uses `USER_HEADER_SEARCH_PATHS` unless `ALWAYS_SEARCH_USER_PATHS` is true
- Only angle uses `SYSTEM_HEADER_SEARCH_PATHS` if the compiler knows the distinction between system and user paths. Otherwise both quote and angle use this.

- `#include "path"` looks in USER_HEADER_SEARCH_PATHS. It then looks in HEADER_SEARCH_PATHS if ALWAYS_SEARCH_USER_PATHS is set.

This is because at first compilers only had `-I<path>`, and that was used for both angle and quote. Then `-iquote` was added
to GCC so that user and system search paths could be distinct, and then `-isystem` was added to remove the need for the
really arcane `-I-` stop-searching-user-paths-now entry.

Here's what happened.

At first, compilers just had `-I<path>`. For GCC, there were true system include paths that were baked into the compiler.
Paths passed with `-I` were searched before the baked-in paths.

Then, `-I-` was added. This was a marker that split the include list in two - everything before this point was only for
quote headers, everything after was for both angle and quote headers. It had another effect which was not standards-compliant: 
seeing `-I-` disabled the "search in current file's directory" behavior.

To fix that, `-iuser <dir>` was added. This list was searched first for quote includes, then only the `-I` list was
searched for both quote and angle includes.

And then `-isystem <dir>` was added as; this list is only searched for angle includes.

**Always Search User Paths (Deprecated) (ALWAYS_SEARCH_USER_PATHS)**

This setting is deprecated as of Xcode 8.3 and may not be supported in future versions. It is recommended that you disable the setting.

If enabled, both #include <header.h>-style and #include "header.h"-style directives search the paths in User Header Search Paths (USER_HEADER_SEARCH_PATHS) before Header Search Paths (HEADER_SEARCH_PATHS). As a consequence, user headers, such as your own String.h header, have precedence over system headers when using #include <header.h>. This is done using the -iquote flag for the paths provided in User Header Search Paths (USER_HEADER_SEARCH_PATHS). If disabled and your compiler fully supports separate user paths, user headers are only accessible with #include "header.h"-style preprocessor directives.

For backwards compatibility reasons, this setting is enabled by default. Disabling it is strongly recommended.

**Header Search Paths (HEADER_SEARCH_PATHS)**

This is a list of paths to folders to be searched by the compiler for included or imported header files when compiling C, Objective-C, C++, or Objective-C++. Paths are delimited by whitespace, so any paths with spaces in them need to be properly quoted.

**System Header Search Paths (SYSTEM_HEADER_SEARCH_PATHS)**

This is a list of paths to folders to be searched by the compiler for included or imported system header files when compiling C, Objective-C, C++, or Objective-C++. The order is from highest to lowest precedence. Paths are delimited by whitespace, so any paths with spaces in them need to be properly quoted. This setting is very similar to "Header Search Paths", except that headers are passed to the compiler in a way that suppresses most warnings for headers found in system search paths. If the compiler doesn't support the concept of system header search paths, then the search paths are appended to any existing header search paths defined in "Header Search Paths".

I think this means it supplants HEADER_SEARCH_PATHS, which was the previous way to set `<>` includes. At the
compiler level, (Clang or GCC) these are passed with `-I<path>`; remember that user-level includes are passed
with `-i<path>`.

**User Header Search Paths (USER_HEADER_SEARCH_PATHS)**

This is a list of paths to folders to be searched by the compiler for included or imported user header files (those headers listed in quotes) when compiling C, Objective-C, C++, or Objective-C++. Paths are delimited by whitespace, so any paths with spaces in them need to be properly quoted. See Always Search User Paths (Deprecated) (ALWAYS_SEARCH_USER_PATHS) for more details on how this setting is used. If the compiler doesn't support the concept of user headers, then the search paths are prepended to the any existing header search paths defined in Header Search Paths (HEADER_SEARCH_PATHS).

Last upgrade check
------------------

Xcode projects have an attribute called `LastUpgradeCheck` in the `PBXProject` section.

```
/* Begin PBXProject section */
		08FB7793FE84155DC02AAC07 /* Project object */ = {
			isa = PBXProject;
			attributes = {
				LastUpgradeCheck = 0910;
			};
			...
		};
/* End PBXProject section */
```

This is the last version of Xcode that ran upgrade checks on this project - for example, the `0910`
value indicates that Xcode 9.1 was the last to upgrade the project. This is an annoying field
that is still sometimes useful; when Xcode changes sufficiently, having it run checks on your project
files to prompt for new desired settings can be useful. It can also be wrong in its suggestions.

If this is missing, then this is assumed to be some value less than `0400`, as that is the first
version of Xcode that had the last-upgrade-check feature.

This check can be disabled via a third-party plugin - see [NoLastUpgradeCheck](https://github.com/0xced/NoLastUpgradeCheck).

Suggested best practice for project generation is to set this to the version of Xcode that you are
generating projects for. This assumes that new projects will be generated for new Xcode versions
rather than having Xcode upgrade them, and that the project generator knows better than Xcode what
the projects should look like.

Compiler versions
-----------------

`-std=c++1z` for the bleeding edge (deprecated in modern clang). Unfortunately, Apple doesn't seem to have Clang 4.0 or Clang 5.0
in Xcode yet.

`llvm.git/tools/clang/include/clang/Frontend/LangStandards.def` has the list of standards that can be
selected from. The live version is `https://github.com/llvm-mirror/clang/blob/master/include/clang/Frontend/LangStandards.def`.

Reference
=========

[GCC Preprocessor: Invocation](https://gcc.gnu.org/onlinedocs/cpp/Invocation.html#Invocation)
