# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/DeadlockSimulator_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/DeadlockSimulator_autogen.dir/ParseCache.txt"
  "DeadlockSimulator_autogen"
  )
endif()
