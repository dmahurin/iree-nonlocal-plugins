// Copyright 2020 The IREE Authors
//
// Licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "driver.h"

#include <stddef.h>
#include <string.h>

#define IREE_HAL_TASK_DEVICE_ID_DEFAULT 0

typedef struct iree_hal_nl_task_driver_t {
  iree_hal_resource_t resource;
  iree_allocator_t host_allocator;

  iree_string_view_t identifier;
  iree_hal_nl_task_device_params_t default_params;

  iree_host_size_t queue_count;
  iree_task_executor_t** queue_executors;

  iree_host_size_t loader_count;
  iree_hal_executable_loader_t* loaders[];
} iree_hal_nl_task_driver_t;

static const iree_hal_driver_vtable_t iree_hal_nl_task_driver_vtable;

static iree_hal_nl_task_driver_t* iree_hal_nl_task_driver_cast(
    iree_hal_driver_t* base_value) {
  IREE_HAL_ASSERT_TYPE(base_value, &iree_hal_nl_task_driver_vtable);
  return (iree_hal_nl_task_driver_t*)base_value;
}

iree_status_t iree_hal_nl_task_driver_create(
    iree_string_view_t identifier,
    const iree_hal_nl_task_device_params_t* default_params,
    iree_host_size_t queue_count, iree_task_executor_t* const* queue_executors,
    iree_host_size_t loader_count, iree_hal_executable_loader_t** loaders,
    iree_allocator_t host_allocator,
    iree_hal_driver_t** out_driver) {
  IREE_ASSERT_ARGUMENT(default_params);
  IREE_ASSERT_ARGUMENT(!queue_count || queue_executors);
  IREE_ASSERT_ARGUMENT(!loader_count || loaders);
  IREE_ASSERT_ARGUMENT(out_driver);
  *out_driver = NULL;
  IREE_TRACE_ZONE_BEGIN(z0);

  // Allocation is for:
  // - iree_hal_nl_task_driver_t
  //   + loaders[] VLA
  // - queue_executors[]
  // - identifier string
  iree_hal_nl_task_driver_t* driver = NULL;
  iree_host_size_t struct_size =
      sizeof(*driver) + loader_count * sizeof(*driver->loaders);
  iree_host_size_t queue_executors_offset = struct_size;
  struct_size += queue_count * sizeof(driver->queue_executors[0]);
  iree_host_size_t identifier_offset = struct_size;
  struct_size += identifier.size;
  iree_status_t status =
      iree_allocator_malloc(host_allocator, struct_size, (void**)&driver);
  if (iree_status_is_ok(status)) {
    iree_hal_resource_initialize(&iree_hal_nl_task_driver_vtable,
                                 &driver->resource);
    driver->host_allocator = host_allocator;

    iree_string_view_append_to_buffer(identifier, &driver->identifier,
                                      (char*)driver + identifier_offset);
    memcpy(&driver->default_params, default_params,
           sizeof(driver->default_params));

    driver->queue_count = queue_count;
    driver->queue_executors =
        (iree_task_executor_t**)((uint8_t*)driver + queue_executors_offset);
    for (iree_host_size_t i = 0; i < driver->queue_count; ++i) {
      driver->queue_executors[i] = queue_executors[i];
      iree_task_executor_retain(driver->queue_executors[i]);
    }

    driver->loader_count = loader_count;
    for (iree_host_size_t i = 0; i < driver->loader_count; ++i) {
      driver->loaders[i] = loaders[i];
      iree_hal_executable_loader_retain(driver->loaders[i]);
    }
  }

  if (iree_status_is_ok(status)) {
    *out_driver = (iree_hal_driver_t*)driver;
  } else {
    iree_hal_driver_release((iree_hal_driver_t*)driver);
  }
  IREE_TRACE_ZONE_END(z0);
  return status;
}

static void iree_hal_nl_task_driver_destroy(iree_hal_driver_t* base_driver) {
  iree_hal_nl_task_driver_t* driver = iree_hal_nl_task_driver_cast(base_driver);
  iree_allocator_t host_allocator = driver->host_allocator;
  IREE_TRACE_ZONE_BEGIN(z0);

  for (iree_host_size_t i = 0; i < driver->loader_count; ++i) {
    iree_hal_executable_loader_release(driver->loaders[i]);
  }
  for (iree_host_size_t i = 0; i < driver->queue_count; ++i) {
    iree_task_executor_release(driver->queue_executors[i]);
  }
  iree_allocator_free(host_allocator, driver);

  IREE_TRACE_ZONE_END(z0);
}

static iree_status_t iree_hal_nl_task_driver_query_available_devices(
    iree_hal_driver_t* base_driver, iree_allocator_t host_allocator,
    iree_host_size_t* out_device_info_count,
    iree_hal_device_info_t** out_device_infos) {
  static const iree_hal_device_info_t device_infos[1] = {
      {
          .device_id = IREE_HAL_TASK_DEVICE_ID_DEFAULT,
          .name = iree_string_view_literal("default"),
      },
  };
  *out_device_info_count = IREE_ARRAYSIZE(device_infos);
  return iree_allocator_clone(
      host_allocator,
      iree_make_const_byte_span(device_infos, sizeof(device_infos)),
      (void**)out_device_infos);
}

static iree_status_t iree_hal_nl_task_driver_dump_device_info(
    iree_hal_driver_t* base_driver, iree_hal_device_id_t device_id,
    iree_string_builder_t* builder) {
  iree_hal_nl_task_driver_t* driver = iree_hal_nl_task_driver_cast(base_driver);
  // TODO(benvanik): dump detailed device info.
  // Could do executable loaders, CPU features, etc. Ideally would share some
  // with the other local drivers.
  (void)driver;
  return iree_ok_status();
}

static iree_status_t iree_hal_nl_task_driver_create_device_by_id(
    iree_hal_driver_t* base_driver, iree_hal_device_id_t device_id,
    iree_host_size_t param_count, const iree_string_pair_t* params,
    iree_allocator_t host_allocator, iree_hal_device_t** out_device) {
  iree_hal_nl_task_driver_t* driver = iree_hal_nl_task_driver_cast(base_driver);
  return iree_hal_nl_task_device_create(
      driver->identifier, &driver->default_params, driver->queue_count,
      driver->queue_executors, driver->loader_count, driver->loaders,
      host_allocator, out_device);
}

static iree_status_t iree_hal_nl_task_driver_create_device_by_path(
    iree_hal_driver_t* base_driver, iree_string_view_t driver_name,
    iree_string_view_t device_path, iree_host_size_t param_count,
    const iree_string_pair_t* params, iree_allocator_t host_allocator,
    iree_hal_device_t** out_device) {
  if (!iree_string_view_is_empty(device_path)) {
    return iree_make_status(IREE_STATUS_NOT_FOUND,
                            "device paths not yet implemented");
  }
  return iree_hal_nl_task_driver_create_device_by_id(
      base_driver, IREE_HAL_DEVICE_ID_DEFAULT, param_count, params,
      host_allocator, out_device);
}

static const iree_hal_driver_vtable_t iree_hal_nl_task_driver_vtable = {
    .destroy = iree_hal_nl_task_driver_destroy,
    .query_available_devices = iree_hal_nl_task_driver_query_available_devices,
    .dump_device_info = iree_hal_nl_task_driver_dump_device_info,
    .create_device_by_id = iree_hal_nl_task_driver_create_device_by_id,
    .create_device_by_path = iree_hal_nl_task_driver_create_device_by_path,
};
