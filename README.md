# sk-cio

(WIP) Coro-based exception-free async I/O library.

* [Documentation](https://sk-cio.readthedocs.io/en/latest/index.html)

Requires:

* [sk-buffer](https://github.com/sikol/sk-buffer)
 
Tested platforms:

* Windows NT, MSVC 19.28 (VS 16.9)
* Linux >= 5.6 with epoll and io_uring, GCC 10.2.0
* Linux < 5.6 with epoll, GCC 10.2.0

Unsupported compilers:

* Clang 11.0 (missing C++20 coroutines)

Example:

* [async_type](sample/async_type.cxx)
* [async_resolve](sample/async_resolve.cxx)
* [async_tcp_echo_server](sample/async_tcp_echo_server.cxx)
