# sk-cio

(WIP) Coro-based exception-free async I/O library.

* [Documentation](https://sk-cio.readthedocs.io/en/latest/index.html)
 
Supported platform / compiler combinations:

* Windows NT, MSVC 19.28 (VS 16.9)
* Windows NT, Clang-cl 11.0.1 (VS 16.9), Microsoft STL
* Linux with epoll and/or io_uring, GCC 10.2.0
* Linux with epoll and/or io_uring, Clang 11.0.1 with libstdc++

Example:

* [async_type](sample/async_type.cxx)
* [async_resolve](sample/async_resolve.cxx)
* [async_tcp_echo_server](sample/async_tcp_echo_server.cxx)
