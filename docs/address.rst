Network addresses
=================

Using network channels requires a way to represent a network address, which is
done with the ``sk::net::address`` class:

.. code-block:: c++

    namespace sk::net {

    template<int address_family>
    struct address;

    }

This class can store an address for any supported protocol, such as IPv4 (INET),
IPv6 (INET6), or UNIX sockets.

``address<>`` is a template class, with one template parameter: the address
family.  For example, ``address<AF_UNIX>`` refers to a UNIX socket address, and
``address<AF_INET6>`` refers to an INET6 address.  This allows type safety when
the type of address is known at compile time:

.. code-block:: c++

    void unix_connect(address<AF_UNIX> const &addr);

    template<int af>
    void tcp_connect(address<af> const &addr) {
        static_assert(af == AF_INET || af == AF_INET6);
        // ...
    }

For brevity, type aliases are available for the supported address types:

.. code-block:: c++

    namespace sk::net {

    using inet_address = address<AF_INET>;
    using inet6_address = address<AF_INET6>;
    using unix_address = address<AF_UNIX>;

    }

When the address type is not known at compile time, which is common when dealing
with IP-based applications that need to support both IPv4 and IPv6, the type-erased
``address<AF_UNSPEC>`` can be used instead.  This type is usually written as ``address<>``.

``address<>`` stores an address of any supported type, and allows the address and
family to be inspected at runtime.

.. code-block:: c++

    template<int af>
    void fn(address<af> const &addr) {
        switch (af) {
        case AF_INET:
        case AF_INET6:
            // We know the address at compile time.
            do_something(af);
            break;

        case AF_UNSPEC: {
            // We can determine the address at runtime.
            int family = address_family(addr);
            do_something(family);
            break;
        }
        }
    }

In most cases, determining the address type like this isn't necessary; ``address_family()``
can be used on any ``address<...>`` object, and will be resolved at either compile
time or runtime as appropriate.

Address types
-------------

INET addresses
^^^^^^^^^^^^^^

An ``inet_address`` represents an IPv4 address, and optionally a port number.

.. code-block:: c++

    namespace sk::net {

    template <>
    struct address<AF_INET> {
        sockaddr_in native_address;

        socklen_t const native_address_length = sizeof(native_address);

        auto port() -> int;
    };

    }

``native_address`` provides access to the native address type.  ``port()`` returns
the port number (in host byte order) or zero if unspecified.

INET6 addresses
^^^^^^^^^^^^^^^

An ``inet6_address`` represents an IPv6 address, and optionally a port number.

.. code-block:: c++

    namespace sk::net {

    template <>
    struct address<AF_INET6> {
        sockaddr_in6 native_address{};

        socklen_t const native_address_length = sizeof(native_address);

        auto port() -> int;
    };

    }

``native_address`` provides access to the native address type.  ``port()`` returns
the port number (in host byte order) or zero if unspecified.

UNIX addresses
^^^^^^^^^^^^^^

A ``unix_address`` represents a UNIX socket address.

.. code-block:: c++

    namespace sk::net {

    template <>
    struct address<AF_UNIX> {
        sockaddr_un native_address{};

        socklen_t const native_address_length = sizeof(native_address);

        std::string path() const;
    };

    }

``native_address`` provides access to the native address type.  ``path()`` returns
the UNIX socket path (``sun_path``).

Creating addresses
------------------

Several functions are available to create a new address:

.. code-block:: c++

    namespace sk::net {

    // Create an IPv4 address.
    auto make_inet_address(std::string const &host, int port = 0)
        -> expected<inet_address, std::error_code>;

    // Create an IPv6 address.
    auto make_inet6_address(std::string const &host, int port = 0)
        -> expected<inet6_address, std::error_code>;

    // Create an AF_UNIX address.
    auto make_unix_address(std::filesystem::path const &path)
        -> expected<unix_address, std::error_code>

    // Create an address of any type.
    auto make_address(std::string const &host,
                      std::string const &service = "")
        -> expected<address<>, std::error_code>

    // Resolve a name to an address at runtime.
    template<int af = AF_UNSPEC>
    auto async_resolve_address(std::string const &hostname,
                               std::string const &port)
        -> task<expected<std::set<address<af>>, std::error_code>>;

    }

``make_xxx_address()`` returns an address from a literal address.  For example,
``make_inet6_address("::1")`` and ``make_unix_address("/tmp/myapp.sock")`` are
valid, but ``make_inet6_address("somehost.example.com")`` is not, because
``somehost.example.com`` is not an address literal.  For INET and INET6
addresses, an optional port number can also be passed; port 0 indicates the
port is not specified.

``make_address()`` returns an ``address<>`` from a literal address; the
specific type of address returned is determined at runtime based on the string
passed.  Only INET and INET6 addresses are supported; UNIX socket paths are not.

``async_resolve_address()`` converts a string into a list of addresses using the
system's address resolution mechanism (usually DNS, and possibly including NIS,
LDAP or other mechanisms).  If the address family is ``AF_UNSPEC``, all addresses
are returned; otherwise, only addresses for the given address family are returned.

Zero addresses
--------------

Many protocols support the concept of a zero address.  For INET and INET6,
this is ``0.0.0.0`` or ``::``.  For UNIX, this is a null (empty) path.

The default value of an address is the zero address:

.. code-block:: c++

    inet_address addr; // str(addr) == "0.0.0.0"

To create a zero address at runtime, use ``make_unspecified_zero_address``:

.. code-block:: c++

    namespace sk::net {

    auto make_unspecified_zero_address(int address_family)
        -> expected<address<>, std::error_code>;

    }

This returns an ``address<>`` containing the zero address for the given
address family.

Printing addresses
------------------

Addresses can be printed to a stream, or converted to an ``std::string`` using
the ``str()`` function:

.. code-block:: c++

    auto a = make_address("::1");

    std::cout << *a << '\n';

    std::string a_str = str(*a);

The address is printed in the conventional manner for the address type:

* For AF_INET, ``1.2.3.4`` or ``1.2.3.4:123``.
* For AF_INET6, ``::1`` or ``[::1]:123``.
* For AF_UNIX, the socket path is used.

Inspecting addresses
--------------------

Two utility functions are available to inspect an address:

.. code-block:: c++

    namespace sk::net {

    template<int af>
    auto address_family(address<af> const &) -> int;

    }

Returns the address family for the given address.

.. code-block:: c++

    namespace sk::net {

    template<int af>
    auto port(address<af> const &) -> expected<int, std::error_code>;

    }

If the address supports the concept of a port or service number (e.g. INET
or INET6), return the port number or zero if unspecified.  Otherwise, returns
an error.

Converting addresses
--------------------

Addresses can be converted between types using ``address_cast()``:

.. code-block:: c++

    namespace sk::net {

    template<typename To, typename From>
    auto address_cast(From const &) -> To;

    }

For example:

.. code-block:: c++

    address<> unspec_address;
    inet_address ia = address_cast<inet_address>(unspec_address);
    if (!ia)
        // Conversion failed...

The following conversions can be performed with compile-time type checking
(meaning if the cast compiles, the result is guaranteed to be a success):

* From any address to ``sockaddr const *``.
* From any address to ``sockaddr_storage const *``.
* From ``inet_address`` to ``sockaddr_in const *``.
* From ``sockaddr_in`` to ``inet_address``.
* From ``inet6_address`` to ``sockaddr_in6 const *``.
* From ``sockaddr_in6`` to ``inet6_address``.
* From ``unix_address`` to ``sockaddr_un const *``.
* From ``sockaddr_un`` to ``unix_address``.

For example:

.. code-block:: c++

    sockaddr_in *addr;

    // Always succeeds
    inet_address = address_cast<inet_address>(*addr);

    // Always succeeds.
    auto caddr = address_cast<sockaddr_in const *>(*inet_address);

The following conversions can be performed with runtime type checking
(so the conversion may fail):

* From ``address<>`` to ``address<af>`` for any ``af``.
* From ``address<af>`` to ``address<>`` for any ``af``.
* From any ``sockaddr``-like type to ``address<>`` or ``address<af>``.
* From ``address<>`` to any ``sockaddr``-like type.

For example:

.. code-block:: c++

    address<> uaddr;

    // Fails if uaddr is not an AF_INET address.
    auto iaddr = address_cast<inet_address>(uaddr);

    sockaddr *saddr;

    // Fails if saddr is not an AF_UNIX address.
    auto unix = address_cast<unix_address>(*saddr);
