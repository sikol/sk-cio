Memory channels
===============

Memory channels allow a region of memory to be accessed as a channel.

There are three memory channel types, all of which support both sequential
and direct access (see :ref:`channel concepts`):

* ``sk::cio::imemchannel`` - read-only access to a memory region.
* ``sk::cio::omemchannel`` - write-only access to a memory region.
* ``sk::cio::memchannel`` - bidirectional access to a memory region.

The ``value_type`` of a memory channel is ``std::byte``.

Include files
-------------

* For ``imemchannel``, include ``<sk/cio/memchannel/imemchannel.hxx>``
* For ``omemchannel``, include ``<sk/cio/memchannel/omemchannel.hxx>``
* For ``memchannel``, include ``<sk/cio/memchannel/memchannel.hxx>``
* For all three types, include ``<sk/cio/memchannel.hxx>`` or ``<sk/cio.hxx>``.

Opening a memory channel
------------------------

Before a memory channel can be used, it must be opened by providing the
address and size of the memory region.  For convenience, overloads are
provided for ``open()`` to allow opening with ``std::byte``, ``char``,
``signed char`` and ``unsigned char``.

``imemchannel`` can be opened with a ``const`` memory region, while
``omemchannel`` and ``memchannel`` require a mutable region.

Opening ``imemchannel``
^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: c++

    auto open(T const *begin, T const *end)
        -> expected<void, std::error_code>;

    auto open(T const *begin, std::size_t size)
        -> expected<void, std::error_code>;

``T`` can be ``std::byte``, ``char``, ``signed char`` or ``unsigned char``.

Opening ``omemchannel`` and ``memchannel``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: c++

    auto open(T *begin, T *end)
        -> expected<void, std::error_code>;

    auto open(T *begin, std::size_t size)
        -> expected<void, std::error_code>;

``T`` can be ``std::byte``, ``char``, ``signed char`` or ``unsigned char``.

Reading and writes memory channels
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Memory channels have similar semantics to files.  The memory channel can be
read or written sequentially until reaching the end of the memory region,
at which point ``sk::cio::error::end_of_file`` will be returned.  Attemping
to write data which would be partially past the end of the region will return
a short read or write.

``memchannel`` uses separate pointers for the sequential read and write
position.  This means you can write data to the channel and then later
read back the same data without adjusting the seek position.
