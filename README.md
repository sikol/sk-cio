# sk-cio

(WIP) Coro-based exception-free async I/O library.

Windows only for now.

* [Documents](https://sk-cio.readthedocs.io/en/latest/index.html)

Requires:

* [sk-buffer](https://github.com/sikol/sk-buffer)
* MSVC 19.28 (VS 16.9) or later.
* Clang is not supported since clang-cl doesn't support standard coroutines (yet).

Example:

* [async_type](sample/async_type.cxx)
