# clib

![GitHub stars](https://img.shields.io/github/stars/sincyn/clib?style=social)
![GitHub forks](https://img.shields.io/github/forks/sincyn/clib?style=social)
![GitHub issues](https://img.shields.io/github/issues/sincyn/clib)
![GitHub license](https://img.shields.io/github/license/sincyn/clib)

clib is
a comprehensive suite of high-performance, cross-compiler compatible C libraries targeting the C17 standard. This
project aims to provide robust, efficient, and easy-to-use implementations for common programming tasks across different
platforms.

## Table of Contents

- [Features](#features)
- [Libraries](#libraries)
- [Requirements](#requirements)
- [Building](#building)
- [Usage](#usage)
- [Examples](#examples)
- [Documentation](#documentation)
- [Contributing](#contributing)
- [Testing](#testing)
- [Performance](#performance)
- [Roadmap](#roadmap)
- [License](#license)
- [Acknowledgements](#acknowledgements)

## Features

- **C17 Standard**: Fully compliant with the latest C17 standard
- **Cross-Platform**: Works seamlessly on Windows, Linux, and macOS
- **High Performance**: Optimized for speed and efficiency
- **Thread-Safe**: Designed for multi-threaded applications
- **Unified Interface**: All libraries combined into a single, easy-to-use interface
- **Extensive Testing**: Comprehensive test suite ensures reliability
- **(Soon to-be) Well-Documented**: Detailed documentation and examples for each library

## Libraries

clib includes the following components, all unified under a single interface:

1. **Socket Library**: Universal socket library for easy network programming
2. **Thread Library**: Cross-platform threading library using pthreads and Windows threads
3. **Log Library**: Efficient logging library with local and remote capabilities
4. **Test Library**: Lightweight yet powerful unit testing framework
5. **Memory Library**: Advanced memory management with custom allocators

## Requirements

- C17 compatible compiler (GCC 11+, Clang 14+, MSVC 2022+)
- CMake 3.12 or higher
- (Optional) OpenSSL for secure networking features

## Building

To build clib, follow these steps:

```bash
git clone https://github.com/sincyn/clib.git
cd clib
mkdir build && cd build
cmake ..
cmake --build .
```

### Build Options

You can customize the build using the following CMake options:

- `-DBUILD_SHARED_LIBS=ON/OFF`: Build shared libraries instead of static (default: OFF)
- `-DBUILD_TESTS=ON/OFF`: Build the test suite (default: ON)
- `-DENABLE_ASAN=ON/OFF`: Enable Address Sanitizer (default: OFF)

Example:

```bash
cmake .. -DBUILD_SHARED_LIBS=ON
```

## Usage

To use clib in your project, you can either:

1. Build and install the library, then link against it in your project
2. Include clib as a submodule in your CMake project

### Option 1: Install and Link

After building, install the library:

```bash
cmake --build . --target install
```

Then, in your project's CMakeLists.txt:

```cmake
find_package(clib REQUIRED)
target_link_libraries(your_target PRIVATE clib::clib_all)
```

### Option 2: CMake Submodule

Add clib as a submodule:

```bash
git submodule add https://github.com/sincyn/clib.git extern/clib
```

In your CMakeLists.txt:

```cmake
add_subdirectory(extern/clib)
target_link_libraries(your_target PRIVATE clib_all)
```

## Examples

Check the `examples/` directory for sample code demonstrating the usage of clib. Here's a quick example using the socket
and log components:

```c
#include <clib.h>

int main() {
    cl_init();

    int server = cl_socket_create_server("127.0.0.1", 8080);
    if (server < 0) {
        cl_log(CL_LOG_ERROR, "Failed to create server");
        return 1;
    }

    cl_log(CL_LOG_INFO, "Server started on port 8080");

    // ... handle connections ...

    cl_socket_close(server);
    cl_cleanup();

    return 0;
}
```

## Documentation

Detailed documentation is a work in progress. C Documentation generator library with treesitter is planned to be used
for generating documentation. There is no ETA for this feature. Please see the examples and include/ directory for more
information on each library.

## Contributing

We welcome contributions to clib! Please see our [Contributing Guide](CONTRIBUTING.md) for details on how to submit pull
requests, report issues, and suggest improvements.

## Testing

clib uses a comprehensive test suite to ensure reliability. To run the tests:

```bash
cd build
ctest
```

## Performance

Performance is a key focus of clib. We aim to provide high-performance implementations that are efficient and scalable.
We regularly benchmark the library to ensure it meets our performance goals. If you encounter any performance issues,
please let us know by opening an issue.

###### **(WIP)** - benchmarks/ directory containing the benchmarking code is planned to be added in the future.

## Roadmap

See the [open issues](https://github.com/sincyn/clib/issues) for a list of proposed features and known issues. Our
long-term goals include:

- Expanding the socket library to support more protocols
- Adding a file I/O library with asynchronous capabilities
- Implementing a cross-platform GUI library

## License

clib is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Acknowledgements

- [OpenSSL](https://www.openssl.org/) for cryptographic functions
- [CMake](https://cmake.org/) for build system generation
- All our [contributors](https://github.com/sincyn/clib/graphs/contributors) who have helped shape this project

---

If you find clib useful, please consider giving it a star on GitHub! Your support helps us continue improving and
maintaining this project.