.. _free functions:

Free I/O functions
==================

These functions build on the basic I/O operations provided by channels
(see :ref:`channel concepts`) to provide more useful and flexible I/O
requests.

Read I/O functions
------------------

``read_some()``, ``async_read_some()``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Read data from a channel.

**Synopsis**:

.. code-block:: c++

    namespace sk::cio {

    // (1)
    template<iseqchannel Channel>
    auto read_some(Channel& channel,
                   channel_value_t<Channel> *buf,
                   io_size_t n)
        -> expected<io_size_t, std::error_code>;

    // (2)
    template<iseqchannel Channel>
    auto async_read_some(Channel& channel,
                         channel_value_t<Channel> *buf,
                         io_size_t n)
        -> task<expected<io_size_t, std::error_code>>;

    // (3)
    template<iseqchannel Channel,
             sk::writable_buffer Buffer>
    auto read_some(Channel &channel,
                   Buffer &buffer,
                   io_size_t n)
        -> expected<io_size_t, std::error_code>
           requires std::same_as<channel_value_t<Channel>,
                                 sk::buffer_value_t<Buffer>>;

    // (4)
    template<iseqchannel Channel,
             sk::writable_buffer Buffer>
    auto async_read_some(Channel &channel,
                         Buffer &buffer,
                         io_size_t n)
         -> task<expected<io_size_t, std::error_code>>
         requires std::same_as<channel_value_t<Channel>,
                               sk::buffer_value_t<Buffer>>;

    // (5)
    template<iseqchannel Channel,
             std::ranges::contiguous_range Range>
    auto read_some(Channel &channel,
                   Range &range,
                   io_size_t n)
         -> expected<io_size_t, std::error_code>
         requires std::same_as<channel_value_t<Channel>,
                               std::ranges::range_value_t<Range>>;

    // (6)
    template<iseqchannel Channel,
             std::ranges::contiguous_range Range>
    auto async_read_some(Channel &channel,
                         Range &range,
                         io_size_t n)
         -> task<expected<io_size_t, std::error_code>>
         requires std::same_as<channel_value_t<Channel>,
                               std::ranges::range_value_t<Range>>;

    } // namespace sk::cio

**Description**:

Read up to ``n`` objects from ``channel``.

**Effects**:

* (1, 2) The objects read are copied to ``buffer``, which must be able
  to hold at least ``n`` objects.
* (3, 4) The objects read are appended to ``buffer``.  If the writable
  capacity of the buffer is smaller than ``n``, objects will be read
  up to the remaining capacity.
* (5, 6) The objects read are copied to ``range``.  If the range is
  smaller than ``n``, objects will be read up to the range's size.

**Return value**:

On success, returns the number of objects read; otherwise, no objects
are read and an error code is returned.

* (1, 3, 5) Once the function returns, the read operation is complete
  and the data is available in the buffer.
* (2, 4, 6) The function returns a ``task<>`` object that should be
  waited on to determine when the read operation is complete.  The
  buffer should not be accessed or modified until the operation has
  completed.

``read_some_at()``, ``async_read_some_at()``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Read data from a channel.

**Synopsis**:

.. code-block:: c++

    namespace sk::cio {

    // (1)
    template<idachannel Channel>
    auto read_some_at(Channel& channel,
                      io_offset_t loc,
                      channel_value_t<Channel> *buf,
                      io_size_t n)
        -> expected<io_size_t, std::error_code>;

    // (2)
    template<idachannel Channel>
    auto async_read_some_at(Channel& channel,
                            io_offset_t loc,
                            channel_value_t<Channel> *buf,
                            io_size_t n)
        -> task<expected<io_size_t, std::error_code>>;

    // (3)
    template<idachannel Channel,
             sk::writable_buffer Buffer>
    auto read_some_at(Channel &channel,
                      io_offset_t loc,
                      Buffer &buffer,
                      io_size_t n)
        -> expected<io_size_t, std::error_code>
           requires std::same_as<channel_value_t<Channel>,
                                  sk::buffer_value_t<Buffer>>;

    // (4)
    template<idachannel Channel,
             sk::writable_buffer Buffer>
    auto async_read_some_at(Channel &channel,
                            io_offset_t loc,
                            Buffer &buffer,
                            io_size_t n)
         -> task<expected<io_size_t, std::error_code>>
         requires std::same_as<channel_value_t<Channel>,
                               sk::buffer_value_t<Buffer>>;

    // (5)
    template<idachannel Channel,
             std::ranges::contiguous_range Range>
    auto read_some_at(Channel &channel,
                      io_offset_t loc,
                      Range &range
                      io_size_t n)
         -> expected<io_size_t, std::error_code>
         requires std::same_as<channel_value_t<Channel>,
                               std::ranges::range_value_t<Range>>;

    // (6)
    template<idachannel Channel,
             std::ranges::contiguous_range Range>
    auto async_read_some_at(Channel &channel,
                         io_offset_t loc
                         Range &range,
                         io_size_t n)
         -> task<expected<io_size_t, std::error_code>>
         requires std::same_as<channel_value_t<Channel>,
                               std::ranges::range_value_t<Range>>;

    } // namespace sk::cio

**Description**:

Read up to ``n`` objects from ``channel`` at storage location ``loc``.

* (1, 2) The objects read are copied to ``buffer``, which must be able
  to hold at least ``n`` objects.
* (3, 4) The objects read are appended to ``buffer``.  If the writable
  capacity of the buffer is smaller than ``n``, objects will be read
  up to the remaining capacity.
* (5, 6) The objects read are copied to ``range``.  If the range is
  smaller than ``n``, objects will be read up to the range's size.

**Return value**:

On success, returns the number of objects read; otherwise, no objects
are read and an error code is returned.

* (1, 3, 5) Once the function returns, the read operation is complete
  and the data is available in the buffer.
* (2, 4, 6) The function returns a ``task<>`` object that should be
  waited on to determine when the read operation is complete.  The
  buffer should not be accessed or modified until the operation has
  completed.

Write I/O functions
-------------------

``write_some()``, ``async_write_some()``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Write data to a channel.

**Synopsis**:

.. code-block:: c++

    namespace sk::cio {

    // (1)
    template<iseqchannel Channel>
    auto write_some(Channel& channel,
                    channel_value_t<Channel> const *buf,
                    io_size_t n)
        -> expected<io_size_t, std::error_code>;

    // (2)
    template<iseqchannel Channel>
    auto async_write_some(Channel& channel,
                          channel_value_t<Channel> const *buf,
                          io_size_t n)
        -> task<expected<io_size_t, std::error_code>>;

    // (3)
    template<iseqchannel Channel,
             sk::readable_buffer Buffer>
    auto write_some(Channel &channel,
                   Buffer &buffer,
                   io_size_t n)
        -> expected<io_size_t, std::error_code>
           requires std::same_as<channel_value_t<Channel>,
                                 sk::buffer_value_t<Buffer>>;

    // (4)
    template<iseqchannel Channel,
             sk::readable_buffer Buffer>
    auto async_write_some(Channel &channel,
                          Buffer &buffer,
                          io_size_t n)
         -> task<expected<io_size_t, std::error_code>>
         requires std::same_as<channel_value_t<Channel>,
                               sk::buffer_value_t<Buffer>>;

    // (5)
    template<iseqchannel Channel,
             std::ranges::contiguous_range Range>
    auto write_some(Channel &channel,
                    Range const &range,
                    io_size_t n)
         -> expected<io_size_t, std::error_code>
         requires std::same_as<channel_value_t<Channel>,
                               std::ranges::range_value_t<Range>>;

    // (6)
    template<iseqchannel Channel,
             std::ranges::contiguous_range Range>
    auto async_write_some(Channel &channel,
                          Range const &range,
                          io_size_t n)
         -> task<expected<io_size_t, std::error_code>>
         requires std::same_as<channel_value_t<Channel>,
                               std::ranges::range_value_t<Range>>;

    } // namespace sk::cio

**Description**:

Write up to ``n`` objects to ``channel``.

**Effects**:

* (1, 2) The objects are written from ``buffer``, which must hold at
  least ``n`` objects.
* (3, 4) The objects are written from ``buffer``.  If the readable
  capacity of the buffer is smaller than ``n``, a number of objects
  will be written up to the remaining capacity.
* (5, 6) The objects are written from ``range``.  If the range is
  smaller than ``n``, a number of objects will be written up to
  the range's size.

Objects are considered written when the data has been sent to the
operating system; the data is not guaranteed to be written to a
storage device (such as a disk or a remote network endpoint) until
the channel is committed using ``commit()`` or ``async_commit()``.

**Return value**:

On success, returns the number of objects written; otherwise, no objects
are written and an error code is returned.

* (1, 3, 5) Once the function returns, the write operation is complete
  and the data has been written.
* (2, 4, 6) The function returns a ``task<>`` object that should be
  waited on to determine when the write operation is complete.  The
  buffer should not be accessed or modified until the operation has
  completed.

``write_all()``, ``async_write_all()``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Write data to a channel.

**Synopsis**:

.. code-block:: c++

    namespace sk::cio {

    // (1)
    template<iseqchannel Channel>
    auto write_all(Channel& channel,
                   channel_value_t<Channel> const *buf,
                   io_size_t n)
        -> pair<io_size_t, std::error_code>;

    // (2)
    template<iseqchannel Channel>
    auto async_write_all(Channel& channel,
                         channel_value_t<Channel> const *buf,
                         io_size_t n)
        -> task<pair<io_size_t, std::error_code>>;

    // (3)
    template<iseqchannel Channel,
             sk::readable_buffer Buffer>
    auto write_all(Channel &channel,
                   Buffer &buffer,
                   io_size_t n)
        -> pair<io_size_t, std::error_code>
           requires std::same_as<channel_value_t<Channel>,
                                 sk::buffer_value_t<Buffer>>;

    // (4)
    template<iseqchannel Channel,
             sk::readable_buffer Buffer>
    auto async_write_all(Channel &channel,
                         Buffer &buffer,
                         io_size_t n)
         -> task<pair<io_size_t, std::error_code>>
         requires std::same_as<channel_value_t<Channel>,
                               sk::buffer_value_t<Buffer>>;

    // (5)
    template<iseqchannel Channel,
             std::ranges::contiguous_range Range>
    auto write_all(Channel &channel,
                   Range const &range,
                   io_size_t n)
         -> pair<io_size_t, std::error_code>
         requires std::same_as<channel_value_t<Channel>,
                               std::ranges::range_value_t<Range>>;

    // (6)
    template<iseqchannel Channel,
             std::ranges::contiguous_range Range>
    auto async_write_all(Channel &channel,
                         Range const &range,
                         io_size_t n)
         -> task<pair<io_size_t, std::error_code>>
         requires std::same_as<channel_value_t<Channel>,
                               std::ranges::range_value_t<Range>>;

    } // namespace sk::cio

**Description**:

Write `up to ```n`` objects to ``channel``.

**Effects**:

* (1, 2) The objects are written from ``buffer``, which must hold at
  least ``n`` objects.
* (3, 4) The objects are written from ``buffer``.  If the readable
  capacity of the buffer is smaller than ``n``, a number of objects
  will be written up to the remaining capacity.
* (5, 6) The objects are written from ``range``.  If the range is
  smaller than ``n``, a number of objects will be written up to
  the range's size.

``write_all()`` and ``async_write_all()`` will attempt to write the
entire contents of the provided buffer, using multiple calls to
``write_some()`` or ``async_write_some()`` if needed.  The number
of objects actually written can still be less than requested if an
error occurs.

Objects are considered written when the data has been sent to the
operating system; the data is not guaranteed to be written to a
storage device (such as a disk or a remote network endpoint) until
the channel is committed using ``commit()`` or ``async_commit()``.

**Return value**:

Returns the number of objects written and the error, if any.

* (1, 3, 5) Once the function returns, the write operation is complete
  and the data has been written.
* (2, 4, 6) The function returns a ``task<>`` object that should be
  waited on to determine when the write operation is complete.  The
  buffer should not be accessed or modified until the operation has
  completed.

``write_some_at()``, ``async_write_some_at()``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Write data to a channel.

**Synopsis**:

.. code-block:: c++

    namespace sk::cio {

    // (1)
    template<idachannel Channel>
    auto write_some_at(Channel& channel,
                       io_offset_t loc,
                       channel_value_t<Channel> const *buf,
                       io_size_t n)
        -> expected<io_size_t, std::error_code>;

    // (2)
    template<idachannel Channel>
    auto async_write_some_at(Channel& channel,
                             io_offset_t loc,
                             channel_value_t<Channel> const *buf,
                             io_size_t n)
        -> task<expected<io_size_t, std::error_code>>;

    // (3)
    template<idachannel Channel,
             sk::readable_buffer Buffer>
    auto write_some_at(Channel &channel,
                       io_offset_t loc,
                       Buffer &buffer,
                       io_size_t n)
        -> expected<io_size_t, std::error_code>
           requires std::same_as<channel_value_t<Channel>,
                                  sk::buffer_value_t<Buffer>>;

    // (4)
    template<idachannel Channel,
             sk::readable_buffer Buffer>
    auto async_write_some_at(Channel &channel,
                             io_offset_t loc,
                             Buffer &buffer,
                             io_size_t n)
         -> task<expected<io_size_t, std::error_code>>
         requires std::same_as<channel_value_t<Channel>,
                               sk::buffer_value_t<Buffer>>;

    // (5)
    template<idachannel Channel,
             std::ranges::contiguous_range Range>
    auto write_some_at(Channel &channel,
                       io_offset_t loc,
                       Range const &range
                       io_size_t n)
         -> expected<io_size_t, std::error_code>
         requires std::same_as<channel_value_t<Channel>,
                               std::ranges::range_value_t<Range>>;

    // (6)
    template<idachannel Channel,
             std::ranges::contiguous_range Range>
    auto async_write_some_at(Channel &channel,
                             io_offset_t loc
                             Range const &range,
                             io_size_t n)
         -> task<expected<io_size_t, std::error_code>>
         requires std::same_as<channel_value_t<Channel>,
                               std::ranges::range_value_t<Range>>;

    } // namespace sk::cio

**Description**:

Write ``n`` objects to ``channel`` at storage location ``loc``.

**Effects**:

* (1, 2) The objects are written from ``buffer``, which must hold at
  least ``n`` objects.
* (3, 4) The objects are written from ``buffer``.  If the readable
  capacity of the buffer is smaller than ``n``, a number of objects
  will be written up to the remaining capacity.
* (5, 6) The objects are written from ``range``.  If the range is
  smaller than ``n``, a number of objects will be written up to
  the range's size.

Objects are considered written when the data has been sent to the
operating system; the data is not guaranteed to be written to a
storage device (such as a disk or a remote network endpoint) until
the channel is committed using ``commit()`` or ``async_commit()``.

**Return value**:

On success, returns the number of objects written; otherwise, no objects
are written and an error code is returned.

* (1, 3, 5) Once the function returns, the write operation is complete
  and the data has been written.
* (2, 4, 6) The function returns a ``task<>`` object that should be
  waited on to determine when the write operation is complete.  The
  buffer should not be accessed or modified until the operation has
  completed.
