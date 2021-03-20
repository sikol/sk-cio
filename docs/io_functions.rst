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
    template<iseqchannel Channel, 
             sk::writable_buffer Buffer>
    auto read_some(Channel &channel,
                   io_size_t n, 
                   Buffer &buffer)
        -> expected<io_size_t, std::error_code>
           requires std::same_as<channel_value_t<Channel>,
                                 sk::buffer_value_t<Buffer>>;

    // (2)
    template<iseqchannel Channel, 
             sk::writable_buffer Buffer>
    auto async_read_some(Channel &channel,
                         io_size_t n, 
                         Buffer &buffer)
         -> task<expected<io_size_t, std::error_code>>
         requires std::same_as<channel_value_t<Channel>,
                               sk::buffer_value_t<Buffer>>;

    // (3)
    template<iseqchannel Channel, 
             std::ranges::contiguous_range Range>
    auto read_some(Channel &channel,
                   io_size_t n, 
                   Range &range)
         -> expected<io_size_t, std::error_code>
         requires std::same_as<channel_value_t<Channel>,
                               std::ranges::range_value_t<Range>>;

    // (4)
    template<iseqchannel Channel, 
             std::ranges::contiguous_range Range>
    auto async_read_some(Channel &channel,
                         io_size_t n, 
                         Range &range)
         -> task<expected<io_size_t, std::error_code>>
         requires std::same_as<channel_value_t<Channel>,
                               std::ranges::range_value_t<Range>>;

    } // namespace sk::cio

**Description**:

Read up to ``n`` objects from ``channel``.  If ``n`` is ``unlimited``,
it will be treated as equal to the remaining space in the destination.

* (1, 2) The objects read are appended to ``buffer``.
* (3, 4) The objects read are copied to ``range``.

**Return value**:

On success, returns the number of objects read; otherwise, no objects
are read and an error code is returned.

``read_some_at()``, ``async_read_some_at()``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Read data from a channel.

**Synopsis**:

.. code-block:: c++

    namespace sk::cio {

    // (1)
    template<idachannel Channel, 
             sk::writable_buffer Buffer>
    auto read_some_at(Channel &channel,
                      io_size_t n, 
                      io_offset_t loc,
                      Buffer &buffer)
        -> expected<io_size_t, std::error_code>
           requires std::same_as<channel_value_t<Channel>,
                                  sk::buffer_value_t<Buffer>>;

    // (2)
    template<idachannel Channel,
             sk::writable_buffer Buffer>
    auto async_read_some_at(Channel &channel,
                            io_size_t n, 
                            io_offset_t loc,
                            Buffer &buffer)
         -> task<expected<io_size_t, std::error_code>>
         requires std::same_as<channel_value_t<Channel>,
                               sk::buffer_value_t<Buffer>>;

    // (3)
    template<idachannel Channel,
             std::ranges::contiguous_range Range>
    auto read_some_at(Channel &channel,
                      io_size_t n, 
                      io_offset_t loc,
                      Range &range)
         -> expected<io_size_t, std::error_code>
         requires std::same_as<channel_value_t<Channel>,
                               std::ranges::range_value_t<Range>>;

    // (4)
    template<idachannel Channel,
             std::ranges::contiguous_range Range>
    auto async_read_some_at(Channel &channel,
                         io_size_t n, 
                         io_offset_t loc,
                         Range &range)
         -> task<expected<io_size_t, std::error_code>>
         requires std::same_as<channel_value_t<Channel>,
                               std::ranges::range_value_t<Range>>;

    } // namespace sk::cio

**Description**:

Read up to ``n`` objects from ``channel`` at storage location ``loc``.
If ``n`` is ``unlimited``, it will be treated as equal to the remaining
space in the destination.

* (1, 2) The objects read are appended to ``buffer``.
* (3, 4) The objects read are copied to ``range``.

**Return value**:

On success, returns the number of objects read; otherwise, no objects
are read and an error code is returned.
