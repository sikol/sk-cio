.. _channel concepts:

Channel concepts
================

A channel concept describes the type of I/O that is supported by a
particular channel.  There are three primary channel concepts:

* Sequential I/O channel (``seqchannel``)

A sequential access channel can be read sequentially from the current
position to the end of the channel.  Some sequential channels (such as
files) allow seeking to a specific position in the channel.  The
majority of typical I/O, including file I/O, is done using sequential
channels.

Sequential channels that represent files are typically not thread-safe,
because accessing the channel changes the internal file pointer.

* Record-oriented sequential I/O channel (``rsqchannel``)

A record-oriented channel is similar to a sequential access channel,
except I/O is done in discrete variable-length units called records;
each I/O operation reads or writes exactly one record, and each
record must be fully read or written in a single operation.

The most common example of a record-oriented channel is a UDP socket.

* Direct access I/O channel (``dachannel``)

A direct access channel can be read or written at any location,
including by different threads concurrently, without seeking or altering
any state.  Direct access channels are typically used by applications
which need to do efficient random I/O, such as database servers and
network storage servers.

Input and output channels
-------------------------

A channel may be an *input channel*, which can only be read from, an
*output channel*, which can only be written to, or a *bidirectional
channel* which supports both reading and writing.

Character types
---------------

A channel can only read and write a particular type of object which
is represented by the channel ``value_type``.  For byte-based binary
I/O, this might be ``std::byte`` or ``unsigned char``.  For character
I/O, it could be ``char``, ``wchar_t``, or a Unicode character type
such as ``char8_t``.

Some channel implementations allow the value type to be chosen when
creating the channel, but others might only support specific types.

Compile-time and runtime polymorphism
-------------------------------------

All basic channel types are *compile-time polymorphic*, which means they
conform to a specific concept and can be used polymorphically via
template arguments.  This provides an efficient call interface with
compile-time binding and no virtual method overhead.

.. code-block:: c++

    // Example of a function that operates on a channel via compile-time
    // polymorphism.
    auto f(iseqchannel auto &chnl) {
        return chnl.write_some(...);
    }

When runtime polymorphism is required (for example, because the type of
channel is not known at compile time), a PMR channel adapter can be used
to convert a channel into an abstract base class type.  This adds a
single virtual function call overhead to each interface function, but in
some cases the compiler may be able to convert this into a non-virtual
call at compile time, leaving no overhead.

The PMR channel types are templated on the value type, but the type can
be fixed at compile time if required.

.. code-block:: c++

    // Example of a function that operates on a PMR channel with runtime
    // polymophism.
    expected<io_size_t, std::error_code>
    f(pmr_iseqchannel<char> &chnl) {
         return chnl.write_some(...);
    }

Basic channel concepts
----------------------

``channel_base``
^^^^^^^^^^^^^^^^

``channel_base`` is the basic channel concept which all channels implement.

The following type is provided:

* ``channel_base::value_type``: the object type supported by this channel,
  such as ``char``.

The following functions are provided:

.. code-block:: c++

  auto is_open() const -> bool;

Return true if this channel is open, otherwise false.

.. code-block:: c++

  auto channel_base::async_close()
       -> task<expected<void, std::error_code>>;

  auto channel_base::close()
       -> expected<void, std::error_code>;

Flush any buffered data and close the channel.  When the channel is
destructed, ``close()`` will be called automatically; depending
on the type of the channel, this might require a blocking operation.
To avoid this, call ``async_close()`` before destructing the channel.

``channel_value_t<>``
^^^^^^^^^^^^^^^^^^^^^

.. code-block:: c++

    template<typename Channel>
    using channel_value_t = typename std::remove_cvref_t<Channel>::value_type;

For a channel type ``C``, ``channel_value_t<C>`` returns the channel's
value type.

``channel_const_value_t<>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: c++

    template<typename Channel>
    using channel_const_value_t =
        typename std::add_const_t<channel_value_t<Channel>>;

For a channel type ``C``, ``channel_const_value_t<C>`` returns the channel's
const value type.

Sequential channel concepts
---------------------------

``oseqchannel``
^^^^^^^^^^^^^^^

``oseqchannel`` is a channel that supports sequential output.

The following functions are provided:

.. code-block:: c++

    auto oseqchannel::write_some(value_type const *buf, io_size_t n)
         -> expected<io_size_t, std::error_code>;

    auto oseqchannel::async_write_some(value_type const *buf, io_size_t n)
         -> task<expected<io_size_t, std::error_code>>;

Write up to ``n`` objects from ``buf`` to the channel and advance the
write pointer by the number of objects written.

``write_some()`` may write up to the entire requested amount, but may
also write less.  On success, returns the number of objects written;
otherwise, no objects are written and an error code is returned.

``iseqchannel``
^^^^^^^^^^^^^^^

``iseqchannel`` is a channel that supports sequential input.

The following functions are provided:

.. code-block:: c++

    auto iseqchannel::read_some(value_type *buf, io_size_t n)
         -> expected<io_size_t, std::error_code>;

    auto iseqchannel::async_read_some(value_type *buf, io_size_t n)
         -> task<expected<io_size_t, std::error_code>>;

Read up to ``n`` objects from the channel into ``buf`` and advance
the read pointer by the number of objects read.

``read_some()`` may read up to the entire entire requested amount,
but may also read less.  On success, returns the number of objects
read; otherwise, no objects are read and an error code is returned.

``seqchannel``
^^^^^^^^^^^^^^

``seqchannel`` is a sequential channel that supports both input
and output.  It provides the interface of both ``iseqchannel`` and
``oseqchannel``.

Record-oriented channel concepts
--------------------------------

``orsqchannel``
^^^^^^^^^^^^^^^

``orsqchannel`` is a channel that supports record-oriented output.

The following functions are provided:

.. code-block:: c++

    auto orsqchannel::write_rec(value_type const *buf, io_size_t n)
         -> expected<void, std::error_code>;

    auto orsqchannel::async_write_rec(value_type const *buf, io_size_t n)
         -> task<expected<void, std::error_code>>;

Write a record consisting of ``n`` objects from ``buf`` to the channel.

If the entire record was written, returns ``error::no_error``.  Otherwise,
the record was not written and an error is returned.

``irsqchannel``
^^^^^^^^^^^^^^^

``irsqchannel`` is a channel that supports record-oriented input.

The following functions are provided:

.. code-block:: c++

    auto irsqchannel::read_rec(value_type *buf, io_size_t n)
         -> expected<io_size_t, std::error_code>;

    auto irsqchannel::async_read_rec(value_type *buf, io_size_t n)
         -> task<expected<io_size_t, std::error_code>>;

Read a record consisting of up to ``n`` objects from the channel into
``buf``.  If ``n`` is not large enough to hold the entire record, the
behaviour depends on the channel type.

If a record was read, the size of the record is returned (minus any
discarded data, if applicable to the channel type).  Otherwise, nothing
is read and an error is returned.

``rsqchannel``
^^^^^^^^^^^^^^

``rsqchannel`` is a record-oriented channel that supports both
input and output.  It provides the interface of both ``irsqchannel``
and ``orsqchannel``.


Direct access channel concepts
------------------------------

``odachannel``
^^^^^^^^^^^^^^

``odachannel`` is a channel that supports direct access output.

The following functions are provided:

.. code-block:: c++

    auto odachannel::write_some_at(io_offset_t loc,
                                   value_type const *buf,
                                   io_size_t n)
         -> expected<io_size_t, std::error_code>;

    auto odachannel::async_write_some_at(io_offset_t loc,
                                         value_type const *buf,
                                         io_size_t n)
         -> task<expected<io_size_t, std::error_code>>;

Write up to ``n`` objects from ``buf`` to the channel at location ``loc``.


``write_some_at()`` may write up to the entire requested amount, but may
also write less.  On success, returns the number of objects written;
otherwise, no objects are written and an error code is returned.

``idachannel``
^^^^^^^^^^^^^^

``idachannel`` is a channel that supports direct access input.

The following functions are provided:

.. code-block:: c++

    auto idachannel::read_some_at(io_offset_t loc,
                                  value_type *buf,
                                  io_size_t n)
         -> expected<io_size_t, std::error_code>;

    auto idachannel::async_read_some_at(io_offset_t loc,
                                        value_type *buf,
                                        io_size_t n)
         -> task<expected<io_size_t, std::error_code>>;

Read up to ``n`` objects from the channel at location ``loc`` into
``buf``.

``read_some_at()`` may read up to the entire entire requested amount,
but may also read less.  On success, returns the number of objects
read; otherwise, no objects are read and an error code is returned.

``dachannel``
^^^^^^^^^^^^^

``dachannel`` is a direct access channel that supports both input
and output.  It provides the interface of both ``idachannel`` and
``odachannel``.

