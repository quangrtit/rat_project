# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 4.0

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /snap/cmake/1468/bin/cmake

# The command to remove a file.
RM = /snap/cmake/1468/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/quang/rat_project/common

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/quang/rat_project/common/build_ubuntu

# Include any dependencies generated for this target.
include CMakeFiles/rat_common.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/rat_common.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/rat_common.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/rat_common.dir/flags.make

CMakeFiles/rat_common.dir/codegen:
.PHONY : CMakeFiles/rat_common.dir/codegen

CMakeFiles/rat_common.dir/src/NetworkManager.cpp.o: CMakeFiles/rat_common.dir/flags.make
CMakeFiles/rat_common.dir/src/NetworkManager.cpp.o: /home/quang/rat_project/common/src/NetworkManager.cpp
CMakeFiles/rat_common.dir/src/NetworkManager.cpp.o: CMakeFiles/rat_common.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/quang/rat_project/common/build_ubuntu/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/rat_common.dir/src/NetworkManager.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/rat_common.dir/src/NetworkManager.cpp.o -MF CMakeFiles/rat_common.dir/src/NetworkManager.cpp.o.d -o CMakeFiles/rat_common.dir/src/NetworkManager.cpp.o -c /home/quang/rat_project/common/src/NetworkManager.cpp

CMakeFiles/rat_common.dir/src/NetworkManager.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/rat_common.dir/src/NetworkManager.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/quang/rat_project/common/src/NetworkManager.cpp > CMakeFiles/rat_common.dir/src/NetworkManager.cpp.i

CMakeFiles/rat_common.dir/src/NetworkManager.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/rat_common.dir/src/NetworkManager.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/quang/rat_project/common/src/NetworkManager.cpp -o CMakeFiles/rat_common.dir/src/NetworkManager.cpp.s

# Object files for target rat_common
rat_common_OBJECTS = \
"CMakeFiles/rat_common.dir/src/NetworkManager.cpp.o"

# External object files for target rat_common
rat_common_EXTERNAL_OBJECTS =

librat_common.a: CMakeFiles/rat_common.dir/src/NetworkManager.cpp.o
librat_common.a: CMakeFiles/rat_common.dir/build.make
librat_common.a: CMakeFiles/rat_common.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/quang/rat_project/common/build_ubuntu/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library librat_common.a"
	$(CMAKE_COMMAND) -P CMakeFiles/rat_common.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/rat_common.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/rat_common.dir/build: librat_common.a
.PHONY : CMakeFiles/rat_common.dir/build

CMakeFiles/rat_common.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/rat_common.dir/cmake_clean.cmake
.PHONY : CMakeFiles/rat_common.dir/clean

CMakeFiles/rat_common.dir/depend:
	cd /home/quang/rat_project/common/build_ubuntu && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/quang/rat_project/common /home/quang/rat_project/common /home/quang/rat_project/common/build_ubuntu /home/quang/rat_project/common/build_ubuntu /home/quang/rat_project/common/build_ubuntu/CMakeFiles/rat_common.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/rat_common.dir/depend

