# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.28

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
CMAKE_COMMAND = /usr/local/cmake/bin/cmake

# The command to remove a file.
RM = /usr/local/cmake/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /rat_project/common/proto

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /rat_project/common/proto/build_centos7

# Include any dependencies generated for this target.
include CMakeFiles/proto.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/proto.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/proto.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/proto.dir/flags.make

Packet.pb.h: /rat_project/common/proto/Packet.proto
Packet.pb.h: /usr/local/protobuf-static/bin/protoc
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --blue --bold --progress-dir=/rat_project/common/proto/build_centos7/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Running cpp protocol buffer compiler on /rat_project/common/proto/Packet.proto"
	/usr/local/protobuf-static/bin/protoc --cpp_out :/rat_project/common/proto/build_centos7 -I /rat_project/common/proto /rat_project/common/proto/Packet.proto

Packet.pb.cc: Packet.pb.h
	@$(CMAKE_COMMAND) -E touch_nocreate Packet.pb.cc

CMakeFiles/proto.dir/Packet.pb.cc.o: CMakeFiles/proto.dir/flags.make
CMakeFiles/proto.dir/Packet.pb.cc.o: Packet.pb.cc
CMakeFiles/proto.dir/Packet.pb.cc.o: CMakeFiles/proto.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/rat_project/common/proto/build_centos7/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/proto.dir/Packet.pb.cc.o"
	/opt/rh/devtoolset-11/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/proto.dir/Packet.pb.cc.o -MF CMakeFiles/proto.dir/Packet.pb.cc.o.d -o CMakeFiles/proto.dir/Packet.pb.cc.o -c /rat_project/common/proto/build_centos7/Packet.pb.cc

CMakeFiles/proto.dir/Packet.pb.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/proto.dir/Packet.pb.cc.i"
	/opt/rh/devtoolset-11/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /rat_project/common/proto/build_centos7/Packet.pb.cc > CMakeFiles/proto.dir/Packet.pb.cc.i

CMakeFiles/proto.dir/Packet.pb.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/proto.dir/Packet.pb.cc.s"
	/opt/rh/devtoolset-11/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /rat_project/common/proto/build_centos7/Packet.pb.cc -o CMakeFiles/proto.dir/Packet.pb.cc.s

# Object files for target proto
proto_OBJECTS = \
"CMakeFiles/proto.dir/Packet.pb.cc.o"

# External object files for target proto
proto_EXTERNAL_OBJECTS =

libproto.a: CMakeFiles/proto.dir/Packet.pb.cc.o
libproto.a: CMakeFiles/proto.dir/build.make
libproto.a: CMakeFiles/proto.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/rat_project/common/proto/build_centos7/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX static library libproto.a"
	$(CMAKE_COMMAND) -P CMakeFiles/proto.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/proto.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/proto.dir/build: libproto.a
.PHONY : CMakeFiles/proto.dir/build

CMakeFiles/proto.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/proto.dir/cmake_clean.cmake
.PHONY : CMakeFiles/proto.dir/clean

CMakeFiles/proto.dir/depend: Packet.pb.cc
CMakeFiles/proto.dir/depend: Packet.pb.h
	cd /rat_project/common/proto/build_centos7 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /rat_project/common/proto /rat_project/common/proto /rat_project/common/proto/build_centos7 /rat_project/common/proto/build_centos7 /rat_project/common/proto/build_centos7/CMakeFiles/proto.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/proto.dir/depend

