# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.6

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /Applications/CLion.app/Contents/bin/cmake/bin/cmake

# The command to remove a file.
RM = /Applications/CLion.app/Contents/bin/cmake/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/makoto/soramitsudev/iroha

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/makoto/soramitsudev/iroha/cmake-build-debug

# Include any dependencies generated for this target.
include core/repository/consensus/CMakeFiles/event_repository.dir/depend.make

# Include the progress variables for this target.
include core/repository/consensus/CMakeFiles/event_repository.dir/progress.make

# Include the compile flags for this target's objects.
include core/repository/consensus/CMakeFiles/event_repository.dir/flags.make

core/repository/consensus/CMakeFiles/event_repository.dir/event_repository.cpp.o: core/repository/consensus/CMakeFiles/event_repository.dir/flags.make
core/repository/consensus/CMakeFiles/event_repository.dir/event_repository.cpp.o: ../core/repository/consensus/event_repository.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/makoto/soramitsudev/iroha/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object core/repository/consensus/CMakeFiles/event_repository.dir/event_repository.cpp.o"
	cd /Users/makoto/soramitsudev/iroha/cmake-build-debug/core/repository/consensus && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/event_repository.dir/event_repository.cpp.o -c /Users/makoto/soramitsudev/iroha/core/repository/consensus/event_repository.cpp

core/repository/consensus/CMakeFiles/event_repository.dir/event_repository.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/event_repository.dir/event_repository.cpp.i"
	cd /Users/makoto/soramitsudev/iroha/cmake-build-debug/core/repository/consensus && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/makoto/soramitsudev/iroha/core/repository/consensus/event_repository.cpp > CMakeFiles/event_repository.dir/event_repository.cpp.i

core/repository/consensus/CMakeFiles/event_repository.dir/event_repository.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/event_repository.dir/event_repository.cpp.s"
	cd /Users/makoto/soramitsudev/iroha/cmake-build-debug/core/repository/consensus && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/makoto/soramitsudev/iroha/core/repository/consensus/event_repository.cpp -o CMakeFiles/event_repository.dir/event_repository.cpp.s

core/repository/consensus/CMakeFiles/event_repository.dir/event_repository.cpp.o.requires:

.PHONY : core/repository/consensus/CMakeFiles/event_repository.dir/event_repository.cpp.o.requires

core/repository/consensus/CMakeFiles/event_repository.dir/event_repository.cpp.o.provides: core/repository/consensus/CMakeFiles/event_repository.dir/event_repository.cpp.o.requires
	$(MAKE) -f core/repository/consensus/CMakeFiles/event_repository.dir/build.make core/repository/consensus/CMakeFiles/event_repository.dir/event_repository.cpp.o.provides.build
.PHONY : core/repository/consensus/CMakeFiles/event_repository.dir/event_repository.cpp.o.provides

core/repository/consensus/CMakeFiles/event_repository.dir/event_repository.cpp.o.provides.build: core/repository/consensus/CMakeFiles/event_repository.dir/event_repository.cpp.o


# Object files for target event_repository
event_repository_OBJECTS = \
"CMakeFiles/event_repository.dir/event_repository.cpp.o"

# External object files for target event_repository
event_repository_EXTERNAL_OBJECTS =

lib/libevent_repository.a: core/repository/consensus/CMakeFiles/event_repository.dir/event_repository.cpp.o
lib/libevent_repository.a: core/repository/consensus/CMakeFiles/event_repository.dir/build.make
lib/libevent_repository.a: core/repository/consensus/CMakeFiles/event_repository.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/makoto/soramitsudev/iroha/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library ../../../lib/libevent_repository.a"
	cd /Users/makoto/soramitsudev/iroha/cmake-build-debug/core/repository/consensus && $(CMAKE_COMMAND) -P CMakeFiles/event_repository.dir/cmake_clean_target.cmake
	cd /Users/makoto/soramitsudev/iroha/cmake-build-debug/core/repository/consensus && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/event_repository.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
core/repository/consensus/CMakeFiles/event_repository.dir/build: lib/libevent_repository.a

.PHONY : core/repository/consensus/CMakeFiles/event_repository.dir/build

core/repository/consensus/CMakeFiles/event_repository.dir/requires: core/repository/consensus/CMakeFiles/event_repository.dir/event_repository.cpp.o.requires

.PHONY : core/repository/consensus/CMakeFiles/event_repository.dir/requires

core/repository/consensus/CMakeFiles/event_repository.dir/clean:
	cd /Users/makoto/soramitsudev/iroha/cmake-build-debug/core/repository/consensus && $(CMAKE_COMMAND) -P CMakeFiles/event_repository.dir/cmake_clean.cmake
.PHONY : core/repository/consensus/CMakeFiles/event_repository.dir/clean

core/repository/consensus/CMakeFiles/event_repository.dir/depend:
	cd /Users/makoto/soramitsudev/iroha/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/makoto/soramitsudev/iroha /Users/makoto/soramitsudev/iroha/core/repository/consensus /Users/makoto/soramitsudev/iroha/cmake-build-debug /Users/makoto/soramitsudev/iroha/cmake-build-debug/core/repository/consensus /Users/makoto/soramitsudev/iroha/cmake-build-debug/core/repository/consensus/CMakeFiles/event_repository.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : core/repository/consensus/CMakeFiles/event_repository.dir/depend
