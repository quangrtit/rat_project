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
CMAKE_SOURCE_DIR = /rat_project/client

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /rat_project/client/build_centos7

# Include any dependencies generated for this target.
include CMakeFiles/rat_client.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/rat_client.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/rat_client.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/rat_client.dir/flags.make

CMakeFiles/rat_client.dir/src/main.cpp.o: CMakeFiles/rat_client.dir/flags.make
CMakeFiles/rat_client.dir/src/main.cpp.o: /rat_project/client/src/main.cpp
CMakeFiles/rat_client.dir/src/main.cpp.o: CMakeFiles/rat_client.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/rat_project/client/build_centos7/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/rat_client.dir/src/main.cpp.o"
	/opt/rh/devtoolset-11/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/rat_client.dir/src/main.cpp.o -MF CMakeFiles/rat_client.dir/src/main.cpp.o.d -o CMakeFiles/rat_client.dir/src/main.cpp.o -c /rat_project/client/src/main.cpp

CMakeFiles/rat_client.dir/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/rat_client.dir/src/main.cpp.i"
	/opt/rh/devtoolset-11/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /rat_project/client/src/main.cpp > CMakeFiles/rat_client.dir/src/main.cpp.i

CMakeFiles/rat_client.dir/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/rat_client.dir/src/main.cpp.s"
	/opt/rh/devtoolset-11/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /rat_project/client/src/main.cpp -o CMakeFiles/rat_client.dir/src/main.cpp.s

CMakeFiles/rat_client.dir/src/Client.cpp.o: CMakeFiles/rat_client.dir/flags.make
CMakeFiles/rat_client.dir/src/Client.cpp.o: /rat_project/client/src/Client.cpp
CMakeFiles/rat_client.dir/src/Client.cpp.o: CMakeFiles/rat_client.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/rat_project/client/build_centos7/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/rat_client.dir/src/Client.cpp.o"
	/opt/rh/devtoolset-11/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/rat_client.dir/src/Client.cpp.o -MF CMakeFiles/rat_client.dir/src/Client.cpp.o.d -o CMakeFiles/rat_client.dir/src/Client.cpp.o -c /rat_project/client/src/Client.cpp

CMakeFiles/rat_client.dir/src/Client.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/rat_client.dir/src/Client.cpp.i"
	/opt/rh/devtoolset-11/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /rat_project/client/src/Client.cpp > CMakeFiles/rat_client.dir/src/Client.cpp.i

CMakeFiles/rat_client.dir/src/Client.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/rat_client.dir/src/Client.cpp.s"
	/opt/rh/devtoolset-11/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /rat_project/client/src/Client.cpp -o CMakeFiles/rat_client.dir/src/Client.cpp.s

CMakeFiles/rat_client.dir/src/FileSender.cpp.o: CMakeFiles/rat_client.dir/flags.make
CMakeFiles/rat_client.dir/src/FileSender.cpp.o: /rat_project/client/src/FileSender.cpp
CMakeFiles/rat_client.dir/src/FileSender.cpp.o: CMakeFiles/rat_client.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/rat_project/client/build_centos7/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/rat_client.dir/src/FileSender.cpp.o"
	/opt/rh/devtoolset-11/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/rat_client.dir/src/FileSender.cpp.o -MF CMakeFiles/rat_client.dir/src/FileSender.cpp.o.d -o CMakeFiles/rat_client.dir/src/FileSender.cpp.o -c /rat_project/client/src/FileSender.cpp

CMakeFiles/rat_client.dir/src/FileSender.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/rat_client.dir/src/FileSender.cpp.i"
	/opt/rh/devtoolset-11/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /rat_project/client/src/FileSender.cpp > CMakeFiles/rat_client.dir/src/FileSender.cpp.i

CMakeFiles/rat_client.dir/src/FileSender.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/rat_client.dir/src/FileSender.cpp.s"
	/opt/rh/devtoolset-11/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /rat_project/client/src/FileSender.cpp -o CMakeFiles/rat_client.dir/src/FileSender.cpp.s

CMakeFiles/rat_client.dir/src/Utils.cpp.o: CMakeFiles/rat_client.dir/flags.make
CMakeFiles/rat_client.dir/src/Utils.cpp.o: /rat_project/client/src/Utils.cpp
CMakeFiles/rat_client.dir/src/Utils.cpp.o: CMakeFiles/rat_client.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/rat_project/client/build_centos7/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/rat_client.dir/src/Utils.cpp.o"
	/opt/rh/devtoolset-11/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/rat_client.dir/src/Utils.cpp.o -MF CMakeFiles/rat_client.dir/src/Utils.cpp.o.d -o CMakeFiles/rat_client.dir/src/Utils.cpp.o -c /rat_project/client/src/Utils.cpp

CMakeFiles/rat_client.dir/src/Utils.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/rat_client.dir/src/Utils.cpp.i"
	/opt/rh/devtoolset-11/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /rat_project/client/src/Utils.cpp > CMakeFiles/rat_client.dir/src/Utils.cpp.i

CMakeFiles/rat_client.dir/src/Utils.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/rat_client.dir/src/Utils.cpp.s"
	/opt/rh/devtoolset-11/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /rat_project/client/src/Utils.cpp -o CMakeFiles/rat_client.dir/src/Utils.cpp.s

CMakeFiles/rat_client.dir/src/ProcessSender.cpp.o: CMakeFiles/rat_client.dir/flags.make
CMakeFiles/rat_client.dir/src/ProcessSender.cpp.o: /rat_project/client/src/ProcessSender.cpp
CMakeFiles/rat_client.dir/src/ProcessSender.cpp.o: CMakeFiles/rat_client.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/rat_project/client/build_centos7/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/rat_client.dir/src/ProcessSender.cpp.o"
	/opt/rh/devtoolset-11/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/rat_client.dir/src/ProcessSender.cpp.o -MF CMakeFiles/rat_client.dir/src/ProcessSender.cpp.o.d -o CMakeFiles/rat_client.dir/src/ProcessSender.cpp.o -c /rat_project/client/src/ProcessSender.cpp

CMakeFiles/rat_client.dir/src/ProcessSender.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/rat_client.dir/src/ProcessSender.cpp.i"
	/opt/rh/devtoolset-11/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /rat_project/client/src/ProcessSender.cpp > CMakeFiles/rat_client.dir/src/ProcessSender.cpp.i

CMakeFiles/rat_client.dir/src/ProcessSender.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/rat_client.dir/src/ProcessSender.cpp.s"
	/opt/rh/devtoolset-11/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /rat_project/client/src/ProcessSender.cpp -o CMakeFiles/rat_client.dir/src/ProcessSender.cpp.s

CMakeFiles/rat_client.dir/src/ProcessUtils.cpp.o: CMakeFiles/rat_client.dir/flags.make
CMakeFiles/rat_client.dir/src/ProcessUtils.cpp.o: /rat_project/client/src/ProcessUtils.cpp
CMakeFiles/rat_client.dir/src/ProcessUtils.cpp.o: CMakeFiles/rat_client.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/rat_project/client/build_centos7/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/rat_client.dir/src/ProcessUtils.cpp.o"
	/opt/rh/devtoolset-11/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/rat_client.dir/src/ProcessUtils.cpp.o -MF CMakeFiles/rat_client.dir/src/ProcessUtils.cpp.o.d -o CMakeFiles/rat_client.dir/src/ProcessUtils.cpp.o -c /rat_project/client/src/ProcessUtils.cpp

CMakeFiles/rat_client.dir/src/ProcessUtils.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/rat_client.dir/src/ProcessUtils.cpp.i"
	/opt/rh/devtoolset-11/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /rat_project/client/src/ProcessUtils.cpp > CMakeFiles/rat_client.dir/src/ProcessUtils.cpp.i

CMakeFiles/rat_client.dir/src/ProcessUtils.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/rat_client.dir/src/ProcessUtils.cpp.s"
	/opt/rh/devtoolset-11/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /rat_project/client/src/ProcessUtils.cpp -o CMakeFiles/rat_client.dir/src/ProcessUtils.cpp.s

CMakeFiles/rat_client.dir/src/FileFolderSender.cpp.o: CMakeFiles/rat_client.dir/flags.make
CMakeFiles/rat_client.dir/src/FileFolderSender.cpp.o: /rat_project/client/src/FileFolderSender.cpp
CMakeFiles/rat_client.dir/src/FileFolderSender.cpp.o: CMakeFiles/rat_client.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/rat_project/client/build_centos7/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object CMakeFiles/rat_client.dir/src/FileFolderSender.cpp.o"
	/opt/rh/devtoolset-11/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/rat_client.dir/src/FileFolderSender.cpp.o -MF CMakeFiles/rat_client.dir/src/FileFolderSender.cpp.o.d -o CMakeFiles/rat_client.dir/src/FileFolderSender.cpp.o -c /rat_project/client/src/FileFolderSender.cpp

CMakeFiles/rat_client.dir/src/FileFolderSender.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/rat_client.dir/src/FileFolderSender.cpp.i"
	/opt/rh/devtoolset-11/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /rat_project/client/src/FileFolderSender.cpp > CMakeFiles/rat_client.dir/src/FileFolderSender.cpp.i

CMakeFiles/rat_client.dir/src/FileFolderSender.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/rat_client.dir/src/FileFolderSender.cpp.s"
	/opt/rh/devtoolset-11/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /rat_project/client/src/FileFolderSender.cpp -o CMakeFiles/rat_client.dir/src/FileFolderSender.cpp.s

CMakeFiles/rat_client.dir/src/FileFolderUtils.cpp.o: CMakeFiles/rat_client.dir/flags.make
CMakeFiles/rat_client.dir/src/FileFolderUtils.cpp.o: /rat_project/client/src/FileFolderUtils.cpp
CMakeFiles/rat_client.dir/src/FileFolderUtils.cpp.o: CMakeFiles/rat_client.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/rat_project/client/build_centos7/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object CMakeFiles/rat_client.dir/src/FileFolderUtils.cpp.o"
	/opt/rh/devtoolset-11/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/rat_client.dir/src/FileFolderUtils.cpp.o -MF CMakeFiles/rat_client.dir/src/FileFolderUtils.cpp.o.d -o CMakeFiles/rat_client.dir/src/FileFolderUtils.cpp.o -c /rat_project/client/src/FileFolderUtils.cpp

CMakeFiles/rat_client.dir/src/FileFolderUtils.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/rat_client.dir/src/FileFolderUtils.cpp.i"
	/opt/rh/devtoolset-11/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /rat_project/client/src/FileFolderUtils.cpp > CMakeFiles/rat_client.dir/src/FileFolderUtils.cpp.i

CMakeFiles/rat_client.dir/src/FileFolderUtils.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/rat_client.dir/src/FileFolderUtils.cpp.s"
	/opt/rh/devtoolset-11/root/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /rat_project/client/src/FileFolderUtils.cpp -o CMakeFiles/rat_client.dir/src/FileFolderUtils.cpp.s

# Object files for target rat_client
rat_client_OBJECTS = \
"CMakeFiles/rat_client.dir/src/main.cpp.o" \
"CMakeFiles/rat_client.dir/src/Client.cpp.o" \
"CMakeFiles/rat_client.dir/src/FileSender.cpp.o" \
"CMakeFiles/rat_client.dir/src/Utils.cpp.o" \
"CMakeFiles/rat_client.dir/src/ProcessSender.cpp.o" \
"CMakeFiles/rat_client.dir/src/ProcessUtils.cpp.o" \
"CMakeFiles/rat_client.dir/src/FileFolderSender.cpp.o" \
"CMakeFiles/rat_client.dir/src/FileFolderUtils.cpp.o"

# External object files for target rat_client
rat_client_EXTERNAL_OBJECTS =

rat_client: CMakeFiles/rat_client.dir/src/main.cpp.o
rat_client: CMakeFiles/rat_client.dir/src/Client.cpp.o
rat_client: CMakeFiles/rat_client.dir/src/FileSender.cpp.o
rat_client: CMakeFiles/rat_client.dir/src/Utils.cpp.o
rat_client: CMakeFiles/rat_client.dir/src/ProcessSender.cpp.o
rat_client: CMakeFiles/rat_client.dir/src/ProcessUtils.cpp.o
rat_client: CMakeFiles/rat_client.dir/src/FileFolderSender.cpp.o
rat_client: CMakeFiles/rat_client.dir/src/FileFolderUtils.cpp.o
rat_client: CMakeFiles/rat_client.dir/build.make
rat_client: /rat_project/client/../common/build_centos7/librat_common.a
rat_client: /rat_project/client/../common/proto/build_centos7/libproto.a
rat_client: /usr/local/lib/libboost_system.a
rat_client: /usr/local/lib/libboost_thread.a
rat_client: /usr/local/openssl-3.0.14/lib64/libssl.a
rat_client: /usr/local/openssl-3.0.14/lib64/libcrypto.a
rat_client: /usr/local/protobuf-static/lib/libprotobuf.a
rat_client: /usr/local/lib/libboost_atomic.a
rat_client: CMakeFiles/rat_client.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/rat_project/client/build_centos7/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Linking CXX executable rat_client"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/rat_client.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/rat_client.dir/build: rat_client
.PHONY : CMakeFiles/rat_client.dir/build

CMakeFiles/rat_client.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/rat_client.dir/cmake_clean.cmake
.PHONY : CMakeFiles/rat_client.dir/clean

CMakeFiles/rat_client.dir/depend:
	cd /rat_project/client/build_centos7 && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /rat_project/client /rat_project/client /rat_project/client/build_centos7 /rat_project/client/build_centos7 /rat_project/client/build_centos7/CMakeFiles/rat_client.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/rat_client.dir/depend

