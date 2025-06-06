################################################################################
# Autogenerated by build_tools/bazel_to_cmake/bazel_to_cmake.py from           #
# runtime/src/iree/hal/drivers/local_task/BUILD.bazel                          #
#                                                                              #
# Use iree_cmake_extra_content from iree/build_defs.oss.bzl to add arbitrary   #
# CMake-only content.                                                          #
#                                                                              #
# To disable autogeneration for this file entirely, delete this header.        #
################################################################################

iree_add_all_subdirs()

option(NL_USE_SERVER "Use NL execution server" OFF)
option(NL_DEBUG "NL Client/server debug level" 0)

if(NL_USE_SERVER)
set(NL_API_CLIENT "../../nonlocal/nl_api_client.c")
else()
set(NL_API_CLIENT "../../nonlocal/nl_api.c")
endif()

iree_cc_library(
  PACKAGE
    drivers::nonlocal_task
  NAME
    nonlocal_task
  HDRS
    "command_buffer.h"
    "device.h"
    "driver.h"
    "event.h"
    "queue.h"
    "queue_state.h"
    "semaphore.h"
    "../../nonlocal/buffer.h"
    "../../nonlocal/allocator.h"
    "../../nonlocal/local_executable.h"
    "../../nonlocal/embedded_elf_loader.h"
    "../../nonlocal/command_data.h"
    "../../nonlocal/nl_api.h"
  SRCS
    "command_buffer.c"
    "device.c"
    "driver.c"
    "event.c"
    "queue.c"
    "queue_state.c"
    "semaphore.c"
    "../../nonlocal/buffer.c"
    "../../nonlocal/allocator.c"
    "../../nonlocal/local_executable.c"
    "../../nonlocal/embedded_elf_loader.c"
    "../../nonlocal/command_data.c"
    "${NL_API_CLIENT}"
  DEPS
    iree::base
    iree::base::internal
    iree::base::internal::arena
    iree::base::internal::cpu
    iree::base::internal::event_pool
    iree::base::internal::synchronization
    iree::base::internal::wait_handle
    iree::hal
    iree::hal::local
    iree::hal::local::executable_environment
    iree::hal::local::executable_library
    iree::hal::utils::deferred_command_buffer
    iree::hal::utils::file_transfer
    iree::hal::utils::resource_set
    iree::hal::utils::semaphore_base
    iree::task
  INCLUDES
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../nonlocal"
  PUBLIC
)

### BAZEL_TO_CMAKE_PRESERVES_ALL_CONTENT_BELOW_THIS_LINE ###
