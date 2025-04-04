# IREE nonlocal HAL driver plugins

### Build

```sh
IREE_BUILD=build
IREE_SRC=iree
PLUGINS_SRC=iree-nonlocal-plugins
CMAKE_BUILD_TYPE=Debug
CMAKE_INSTALL_PREFIX=/usr/local
IREE_EXTERNAL_HAL_DRIVERS="nonlocal-sync;nonlocal-task"

cmake -G Ninja -B "${IREE_BUILD}" -S "${IREE_SRC}" \
	-DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
	-DCMAKE_C_COMPILER=clang \
	-DCMAKE_CXX_COMPILER=clang++ \
	-DIREE_CMAKE_BUILTIN_PLUGIN_PATHS="${PLUGINS_SRC}" \
	-DIREE_EXTERNAL_HAL_DRIVERS="${IREE_EXTERNAL_HAL_DRIVERS}" \
	-DIREE_BUILD_PYTHON_BINDINGS=ON  \
	-DNL_USE_SERVER=ON \
	-DNL_DEBUG=1 \
	-DPython3_EXECUTABLE="$(which python3)" \
	-DCMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}"

cmake --build "${IREE_BUILD}"
```

### Run server

```sh
"${IREE_BUILD}"/runtime/plugins/hal/drivers/psm-device-sync/elf_module_server &
```

### Run test client
"${IREE_BUILD}"/runtime/plugins/hal/drivers/psm-device-sync/elf_module_test_client

# Compile/Run MLIR

```sh
iree-compile \
  --iree-hal-target-device=local --iree-hal-target-backends=llvm-cpu \
  "${IREE_SRC}"/samples/models/simple_abs.mlir \
  -o /tmp/simple_abs.vmfb
```

```sh
iree-run-module --module=/tmp/simple_abs.vmfb --device=nonlocal-task \
  --function=abs --input=f32=-2
```
