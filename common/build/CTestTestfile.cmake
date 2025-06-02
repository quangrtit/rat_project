# CMake generated Testfile for 
# Source directory: /home/quang/rat_project/common
# Build directory: /home/quang/rat_project/common/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[RATTests]=] "/home/quang/rat_project/common/build/rat_tests")
set_tests_properties([=[RATTests]=] PROPERTIES  _BACKTRACE_TRIPLES "/home/quang/rat_project/common/CMakeLists.txt;51;add_test;/home/quang/rat_project/common/CMakeLists.txt;0;")
subdirs("proto")
