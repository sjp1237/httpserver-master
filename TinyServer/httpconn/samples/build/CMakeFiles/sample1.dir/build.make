# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.18

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
CMAKE_COMMAND = /opt/cmake-3.18.0/bin/cmake

# The command to remove a file.
RM = /opt/cmake-3.18.0/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/sjp/TinyWebserver/httpconn/samples

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/sjp/TinyWebserver/httpconn/samples/build

# Include any dependencies generated for this target.
include CMakeFiles/sample1.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/sample1.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/sample1.dir/flags.make

CMakeFiles/sample1.dir/sample_request_unittest.cc.o: CMakeFiles/sample1.dir/flags.make
CMakeFiles/sample1.dir/sample_request_unittest.cc.o: ../sample_request_unittest.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/sjp/TinyWebserver/httpconn/samples/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/sample1.dir/sample_request_unittest.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/sample1.dir/sample_request_unittest.cc.o -c /home/sjp/TinyWebserver/httpconn/samples/sample_request_unittest.cc

CMakeFiles/sample1.dir/sample_request_unittest.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sample1.dir/sample_request_unittest.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sjp/TinyWebserver/httpconn/samples/sample_request_unittest.cc > CMakeFiles/sample1.dir/sample_request_unittest.cc.i

CMakeFiles/sample1.dir/sample_request_unittest.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sample1.dir/sample_request_unittest.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sjp/TinyWebserver/httpconn/samples/sample_request_unittest.cc -o CMakeFiles/sample1.dir/sample_request_unittest.cc.s

CMakeFiles/sample1.dir/home/sjp/TinyWebserver/httpconn/httpconn.cc.o: CMakeFiles/sample1.dir/flags.make
CMakeFiles/sample1.dir/home/sjp/TinyWebserver/httpconn/httpconn.cc.o: /home/sjp/TinyWebserver/httpconn/httpconn.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/sjp/TinyWebserver/httpconn/samples/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/sample1.dir/home/sjp/TinyWebserver/httpconn/httpconn.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/sample1.dir/home/sjp/TinyWebserver/httpconn/httpconn.cc.o -c /home/sjp/TinyWebserver/httpconn/httpconn.cc

CMakeFiles/sample1.dir/home/sjp/TinyWebserver/httpconn/httpconn.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sample1.dir/home/sjp/TinyWebserver/httpconn/httpconn.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sjp/TinyWebserver/httpconn/httpconn.cc > CMakeFiles/sample1.dir/home/sjp/TinyWebserver/httpconn/httpconn.cc.i

CMakeFiles/sample1.dir/home/sjp/TinyWebserver/httpconn/httpconn.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sample1.dir/home/sjp/TinyWebserver/httpconn/httpconn.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sjp/TinyWebserver/httpconn/httpconn.cc -o CMakeFiles/sample1.dir/home/sjp/TinyWebserver/httpconn/httpconn.cc.s

CMakeFiles/sample1.dir/home/sjp/TinyWebserver/tool/tool.cc.o: CMakeFiles/sample1.dir/flags.make
CMakeFiles/sample1.dir/home/sjp/TinyWebserver/tool/tool.cc.o: /home/sjp/TinyWebserver/tool/tool.cc
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/sjp/TinyWebserver/httpconn/samples/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/sample1.dir/home/sjp/TinyWebserver/tool/tool.cc.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/sample1.dir/home/sjp/TinyWebserver/tool/tool.cc.o -c /home/sjp/TinyWebserver/tool/tool.cc

CMakeFiles/sample1.dir/home/sjp/TinyWebserver/tool/tool.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/sample1.dir/home/sjp/TinyWebserver/tool/tool.cc.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sjp/TinyWebserver/tool/tool.cc > CMakeFiles/sample1.dir/home/sjp/TinyWebserver/tool/tool.cc.i

CMakeFiles/sample1.dir/home/sjp/TinyWebserver/tool/tool.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/sample1.dir/home/sjp/TinyWebserver/tool/tool.cc.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sjp/TinyWebserver/tool/tool.cc -o CMakeFiles/sample1.dir/home/sjp/TinyWebserver/tool/tool.cc.s

# Object files for target sample1
sample1_OBJECTS = \
"CMakeFiles/sample1.dir/sample_request_unittest.cc.o" \
"CMakeFiles/sample1.dir/home/sjp/TinyWebserver/httpconn/httpconn.cc.o" \
"CMakeFiles/sample1.dir/home/sjp/TinyWebserver/tool/tool.cc.o"

# External object files for target sample1
sample1_EXTERNAL_OBJECTS =

sample1: CMakeFiles/sample1.dir/sample_request_unittest.cc.o
sample1: CMakeFiles/sample1.dir/home/sjp/TinyWebserver/httpconn/httpconn.cc.o
sample1: CMakeFiles/sample1.dir/home/sjp/TinyWebserver/tool/tool.cc.o
sample1: CMakeFiles/sample1.dir/build.make
sample1: CMakeFiles/sample1.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/sjp/TinyWebserver/httpconn/samples/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX executable sample1"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/sample1.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/sample1.dir/build: sample1

.PHONY : CMakeFiles/sample1.dir/build

CMakeFiles/sample1.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/sample1.dir/cmake_clean.cmake
.PHONY : CMakeFiles/sample1.dir/clean

CMakeFiles/sample1.dir/depend:
	cd /home/sjp/TinyWebserver/httpconn/samples/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/sjp/TinyWebserver/httpconn/samples /home/sjp/TinyWebserver/httpconn/samples /home/sjp/TinyWebserver/httpconn/samples/build /home/sjp/TinyWebserver/httpconn/samples/build /home/sjp/TinyWebserver/httpconn/samples/build/CMakeFiles/sample1.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/sample1.dir/depend

