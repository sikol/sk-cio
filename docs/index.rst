sk-cio: Coro-based exception-free async I/O library 
===================================================

sk-cio (Channel I/O) is an I/O library for C++.  It provides a unified interface
for accessing various I/O devices, including files and network sockets, with
both a synchronous API and a coroutine-based asynchronous API.

.. toctree::
   :maxdepth: 3
   :caption: Contents:

   license.rst
   expected.rst
   concepts.rst
   io_functions.rst
   filechannel.rst

Supported compilers
-------------------

sk-config is tested with:

* MSVC 19.28 (VS 16.9) or later.
* Clang-cl is not supported since it doesn't support standard coroutines with
  the Microsoft STL (yet).

Indices and tables
------------------

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
