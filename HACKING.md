﻿# Development HOWTO

This document describes the development process we're following. It's required reading for anyone intending to contribute.

Dependencies
---------------
Main dependencies:
* [SDL2](https://www.libsdl.org/download-2.0.php) — crossplatform media framework;
* [FFmpeg](https://ffmpeg.zeranoe.com/builds/) — video support;
* [OpenAL](https://www.openal.org/downloads/OpenAL11CoreSDK.zip) — audio support;
* [Zlib](http://gnuwin32.sourceforge.net/packages/zlib.htm) — compression.

On Windows, the above dependencies are resolved automatically during the cmake phase.

Additional dependencies:
* CMake 3.20.4+ (3.20.21032501-MSVC_2 from VS2019 won't work)
* Python 3.x (optional, for style checks)

Minimum required compiler versions are as follows:
* Visual Studio 2022
* GCC 11
* Clang 13

The following IDEs have been tested and should work fine:
* Visual Studio (2022 or later)
* Visual Studio Code (2022 or later)
* CLion (2022 or later)

Building on \*nix platforms
---------------
This project uses the [CMake](https://cmake.org) build system.
Use the following commands to clone repository and build (it is recommended to build in a separate directory as shown here):

```
$ git clone https://github.com/OpenEnroth/OpenEnroth.git
$ cd OpenEnroth
$ cmake -B build -S .
$ cmake --build build
```

To compile x86 build on x86_64 platform you can pass `-m32` via compiler flags to cmake.
For example instead of just `cmake ..` above execute `CFLAGS="-m32" CXXFLAGS="-m32" cmake ..`

You can also select platform dependent [generator](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html) for your favorite IDE.

Building on Windows
---------------

* Get git (`https://git-scm.com/download/win`) and Visual Studio 2022
* Make sure you have Windows SDK v10.0.20348.0 or higher
* Clone, fork or download the repo `https://github.com/OpenEnroth/OpenEnroth`
* Setup Cmake:
  * either install standalone cmake from the official website,
  * or add Microsoft one (that's coming with the VS installation) to your PATH environment variable (e.g `c:\Program Files\Microsoft Visual Studio\2022\<edition>\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin`) 
* Open the folder in Visual Studio
* Select build configuration (x32 or x64) and wait for CMake configuration to complete
* Select startup item as `OpenEnroth.exe`
* Run!

If you wish you can also disable autoresolving main dependencies by turning off `PREBUILT_DEPENDENCIES` cmake option and pass your own dependencies source, e.g. via [vcpkg](https://github.com/microsoft/vcpkg) integration.

__Be aware__ that Visual Studio has a bug with git submodules not syncing between branches.
So when checking out the branch or switching to different branch you may need to run the following command manually: `git submodule update --init`

Project Resources
---------------
The Git repo contains some additional resources required for the engine to run.
Please copy the entire 'shaders' folder from the 'resources' subdirectory to the location of the game assets.
Please check the logger output for the required path if you are unsure.

Support
---------------
Still having problems? Ask for help on our discord! [![](https://img.shields.io/badge/chat-on%20discord-green.svg)](https://discord.gg/jRCyPtq)


Coding style
---------------
For the C++ code we are following the [Google C++ Style Guide](http://google.github.io/styleguide/cppguide.html).\
Source code is automatically checked against it and Pull Request will fail if you don't follow it.

To perform style check before pushing anything you can build `check_style` target. In Visual Studio you can do that by going to ***Solution Explorer → Change Views → CMake targets***. Right click and build `check_style`, errors will be listed in output.

We also follow some additional style preferences, as listed below.

Documentation:
* Documentation should be in doxydoc format with `@` used for tags, and starting with `/**` comment introducer.
* Documentation should be written in English.
* Please leave original function offsets intact. If you have a chance, move them to doxygen comments using `@offset` doxygen tag (e.g. `@offset 0xCAFEBEEF`).

Naming:
* Use `MM_` prefix for macro naming. Macro names should be in `SNAKE_CASE_ALL_CAPS`.
* Use `SNAKE_CASE_ALL_CAPS` for `enum` values. E.g. `ITEM_CRATE_OF_ARROWHEADS`, `ITEM_SLOT_RING6`.
* Use `CamelCase` for everything else.
* Type names should start with a capital letter. E.g. `IndexedArray`, `InputAction`, `PlatformLogLevel`. This applies to all types, including classes, structs, enums and typedefs, with certain exceptions as listed below.
* Method & function names should start with a lowercase letter. E.g. `Vec3::length`, `gridCellToWorldPosX`, `getCeilingHeight`.
* Variable names should start with a lowercase letter. E.g. `int monsterCount = level->monsterCount()`.
* Names of private members should start with an underscore to visually distinguish them from variables without having to spell out `this->` every single time. E.g. `_initialized = true`, where `_initialized` is a member field.
* Note that the above doesn't apply to POD-like types as for such types all members are public and are named just like ordinary variables.
* Exceptions to the rules above are STL-compatible interfaces, which should follow STL naming rules. So it's `value_type` for iterator value type, and `push_back` for a method that's adding an element to a container.

Code formatting:
* `*` and `&` in type declarations should be preceded by a space. So it's `char *string`, and not `char* string`.

Language features:
* Use `enum class`es followed by `using enum` statements instead of ordinary `enum`s. This provides type safety without changing the syntax. For flags, use `Flags` class.
* It's OK to use plain `enum`s if you really need to have implicit casts to integer types, but this is a very rare use case. If you're using `enum` values to index into some array, consider using `enum class` coupled with `IndexedArray`.
* Make your code speak for itself when it comes to ownership. If a function takes ownership of one of its parameters, it should take `std::unique_ptr` by value. If it allocates its result and passes ownership to the caller, then it should return `std::unique_ptr`.

There is a lot of code in the project that doesn't follow these conventions. Please feel free to fix it, preferably not mixing up style and logical changes in the same PR.

Testing
-------
We strive for a good test coverage of the project, and while we're not there yet, the current policy is to add tests for all the bugs we fix, as long as the fix is testable. E.g. graphical glitches are generally very hard to test, but we have the infractructure to test game logic and small isolated classes.

Tests in OpenEnroth fall into two categories:
* Unit tests. These are a standard breed of tests, written using Google Test. You can see some examples in `src/Utility/Tests`.
* Game tests. If you're familiar with how testing is done these days for complex mobile apps, then you can consider game tests a variation of UI tests that's specific to our project. Game tests need game assets to run, and are currently **not** run in github actions.

Game tests work by instrumenting the engine, and then running test code in between the game frames. This code usually sends events to the engine (e.g. mouse clicks), which are then processed by the engine in the next frame, but it can do pretty much anything else – all of engine's data is accessible and writable from inside the game test.

As typing out all the events to send inside the test code can be pretty tedious, we also have an event recording functionality, so that the test creation workflow usually looks as follows:
* Load a save that reproduces the bug that you've just fixed.
* Press `Ctrl+Shift+R` to start recording an event trace. Check logs to make sure that trace recording has started.
* Perform the steps that used to reproduce the bug.
* Press `Ctrl+Shift+R` again to stop trace recording. You will get two files generated in the current folder – `trace.json` and `trace.mm7`.
* Rename them into something more suiting (e.g. `issue_XXX.json` and `issue_XXX.mm7`) and create a PR to the [OpenEnroth_TestData](https://github.com/OpenEnroth/OpenEnroth_TestData) repo. 
* Once it's merged update the reference tag in the corresponding [CMakeLists.txt](https://github.com/OpenEnroth/OpenEnroth/blob/master/test/Bin/GameTest/CMakeLists.txt#L21) in the main repo.
* Create a new test case in one of the game test files in [the game tests folder](https://github.com/OpenEnroth/OpenEnroth/tree/master/test/Tests).
* Use `TestController::playTraceFromTestData` to play back your trace, and add the necessary checks around it. If you're simply testing that the test doesn't crash, then just playing back the trace shall be enough.

To run all unit tests locally, build a `UnitTest` cmake target, or build & run `OpenEnroth_UnitTest`.

To run all game tests locally, set `OPENENROTH_MM7_PATH` environment variable to point to the location of the game assets, then build `GameTest` cmake target. Alternatively, you can build `OpenEnroth_GameTest`, and run it manually, passing the paths to both game assets and the test data via command line.


Additional Resources
--------------------

Old event decompiler and IDB files can be found [here](https://www.dropbox.com/sh/if4u3lphn633oit/AADUYMxNcrkAU6epJ50RskyXa?dl=0). Feel free to ping `zipi#6029` for more info.
