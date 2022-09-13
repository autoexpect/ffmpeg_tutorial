# this is required
set(CMAKE_SYSTEM_NAME Linux)

# search for programs in the build host directories (not necessary)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
# set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
# set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

include_directories(${ROOT_DIR}/target/lib)
link_directories(${ROOT_DIR}/target/include)
