Error handling and ``expected<>``
=================================

Many I/O functions can return error conditions; for example, "file not found",
"connection refused", and so on.  Sometimes a function can return either an
error or a value: a function that reads from a file could return the number
of bytes read, or it could return an error.

To represent error conditions, functions in CIO that can encounter errors
return ``expected<T, std::error_code>``, where ``T`` is the actual result
of the function (which could be ``void``), and ``std::error_code`` represents
the error, if any.

Checking for errors is straightforward.  For example, if we have a function
called ``some_function()`` which returns ``expected<int, std::error_code>``:

.. code-block:: c++

    auto ret = some_function(); // ret == expected<int, std::error_code>
    if (ret)
        std::cout << "the value is " << *ret << '\n';
    else
        std::cerr << "an error occured: " << ret.error().message() << '\n';

The ``expected<>`` object should be tested to determine if the operation was
successful or not.  In the success case, dereferencing the ``expected<>`` 
object returns the value.  In the error case, calling the ``.error()`` member
returns the error condition; in the case of ``std::error_code``, the 
``.message()`` member can then be used to retrieved the human-readable error.

For a function that returns ``expected<void, std::error_code>``, the error
path is the same, but ``ret`` cannot be dereferenced since there is no
value.
