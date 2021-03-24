File channels
=============

File channels allow reading from and writing to files.  There are
six file channel types:

* ``iseqfilechannel``: Sequential input (read) file channel
* ``oseqfilechannel``: Sequential output (write) file channel
* ``seqfilechannel``: Sequential bidirectional file channel
* ``idafilechannel``: Direct access input (read) file channel
* ``odafilechannel``: Direct access output (write) file channel
* ``dafilechannel``: Direct access bidirectional file channel

The ``value_type`` of a filechannel is ``std::byte``.

Include files
-------------

* For ``iseqfilechannel``, include ``<sk/cio/filechannel/iseqfilechannel.hxx>``.
* For ``oseqfilechannel``, include ``<sk/cio/filechannel/oseqfilechannel.hxx>``.
* For ``seqfilechannel``, include ``<sk/cio/filechannel/seqfilechannel.hxx>``.
* For ``idafilechannel``, include ``<sk/cio/filechannel/idafilechannel.hxx>``.
* For ``odafilechannel``, include ``<sk/cio/filechannel/odafilechannel.hxx>``.
* For ``dafilechannel``, include ``<sk/cio/filechannel/dafilechannel.hxx>``.

To include all types, include ``<sk/ci/filechannel.hxx>`` or ``<sk/cio.hxx>``.

Opening files
-------------

Before a file channel can be used, it must be opened to associate the
channel with a file.  Use the ``open()`` or ``async_open()`` functions
to do this:

.. code-block:: c++

    auto async_open(std::filesystem::path const &,
                    fileflags_t = fileflags::none)
        -> task<expected<void, std::error_code>>;

    auto open(std::filesystem::path const &,
                fileflags_t = fileflags::none)
        -> expected<void, std::error_code>;

``path`` is the filename of the file to be opened.

``flags`` is one or more of the following logical flags:

* ``fileflags::none``: No effect.
* ``fileflags::read``: Open the file for reading.
* ``fileflags::write``: Open the file for writing.
* ``fileflags::trunc``: When opening a file for writing, if the file
  already exists, truncate it to zero length.
* ``fileflags::append``: When opening a file for writing, if the file
  already exists, seek to the end of the file. (Sequential file channels
  only.)
* ``fileflags::create_new``: When opening a file for writing, if the
  file does not already exist, create a new file.
* ``fileflags::open_existing``: When opening a file for writing, if the
  file already exists, open the existing file.

``read`` and ``write`` are implied by the type of the file channel
(input or output) and do not need to be specified.

If ``write`` is not set, then ``trunc``, ``append``, ``create_new`` and
``open_existing`` must not be set.

If ``write`` is set, at least one of ``create_new`` or ``open_existing``
must be set.
