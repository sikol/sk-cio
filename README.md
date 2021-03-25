# sk-cio

(WIP) Coro-based exception-free async I/O library.

* [Documentation](https://sk-cio.readthedocs.io/en/latest/index.html)

Requires:

* [sk-buffer](https://github.com/sikol/sk-buffer)
  
Tested compilers:
* MSVC 19.28 (VS 16.9)
* GCC 10.2.0
  
Unsupported compilers:

* Clang 11.0 (missing C++20 coroutines)

Example:

* [async_type](sample/async_type.cxx)
* [async_resolve](sample/async_resolve.cxx)
