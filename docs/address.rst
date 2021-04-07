Network addresses
=================

Using network channels requires a way to represent a network address, which is
done with the *address* and *endpoint* types.

Unless otherwise specified, all symbols described here are defined in ``<sk/net/address.hxx>``.

.. cpp:concept:: template<typename T> sk::net::address_family

    A concept that describes an address family.

    ``address_family`` represents the characteristics of a particular address family,
    such as IPv4 (INET), IPv6 (INET6), or UNIX sockets.  The address family types are
    typically not used directly, but provide the template argument for ``address<>``.

.. cpp:type:: sk::net::address_family_tag = implementation_defined

    An integer type that represents an address family.

.. cpp:struct:: sk::net::inet_family

    The inet (IPv4) address family.

.. cpp:struct:: sk::net::inet6_family

    The inet6 (IPv6) address family.

.. cpp:struct:: sk::net::unix_family

    The UNIX socket address family.

.. cpp:struct:: sk::net::unspecified_family

    The unspecified address family (described below).

.. cpp:class:: template<address_family af = unspecified_family> sk::net::address

    An address.

    ``address<>`` represents a single network address, such as an IP address or UNIX path.
    It can be templated on an address family, such as ``address<inet_family>``, or the
    type-erased ``address<>`` can be used to store any kind of address (providing runtime
    polymorphism over address type).

.. cpp:class:: sk::net::tcp_endpoint
.. cpp:class:: sk::net::udp_endpoint
.. cpp:class:: sk::net::unix_endpoint

    ``tcp_endpoint``, ``udp_endpoint`` and ``unix_endpoint`` represent a combination of a
    network address and any additional details required for a network connection, such as
    the port number for TCP and UDP.

Working with addresses
----------------------

Some generic functions are provided for working with address types.

.. cpp:function:: template <address_family af> \
                  auto sk::net::tag(address<af> const &) noexcept -> address_family_tag

    Return the address tag for an address.  For ``address<>``, this is determined at runtime,
    otherwise at compile time.  The address tag can be used to determine the address family,
    by comparing it to a tag constant such as ``inet_family::tag``.

.. cpp:function:: template <address_family family> \
                  auto sk::net::socket_address_family(address<family> const &) -> int

    Return the socket address family for an address, e.g. ``AF_INET`` or ``AF_UNIX``.

.. cpp:function:: template<address_family af> \
                  auto sk::net::str(address<af> const &) -> std::string

    Convert an address to a string in the canonical format.  For INET and INET6, this is the
    standard IP address representation; for UNIX addresses, it is the path.

.. cpp:function:: template <address_family family> \
                  auto sk::net::operator<<(std::ostream &, address<family> const &) -> std::ostream &

    Print ``str(addr)`` to ``strm``.

.. cpp:function:: template <typename To, typename From> \
                  auto sk::net::address_cast(From &&from) -> expected<To, std::error_code>

    Convert one address type to another (described below).

.. cpp:function:: template <address_family af1, address_family af2> \
                  bool sk::net::operator==(address<af1> const &a, address<af2> const &b)

    Compare addresses for ordering.

.. cpp:function:: template <address_family af1, address_family af2> \
                  bool sk::net::operator<(address<af1> const &a, address<af2> const &b)

    Compare addresses for equality.

Address types
-------------

INET addresses
^^^^^^^^^^^^^^

An ``inet_address`` represents an IPv4 address.

.. code-block:: c++


        struct inet_family {
            static constexpr address_family_tag tag = /* implementation-defined */;

            static constexpr std::size_t address_size = 4;
            struct address_type {
                std::array<std::uint8_t, address_size> bytes;
            };
        };

        template <>
        class address<inet_family> {
            using address_family = inet_family;
            using address_type = address_family::address_type;

            address() noexcept;
            address(address_type const &a) : _address(a) {}
            address(address const &other) noexcept;
            auto operator=(address const &other) noexcept -> address &;

            auto value() noexcept -> address_type &
            auto value() const noexcept -> address_type const &
            auto as_bytes() const noexcept
                -> std::span<std::byte const, inet_family::address_size>
        };

    }

A default-constructed ``inet_address`` stores the zero address (``0.0.0.0``).

``value()`` returns the stored address as an array of bytes.  ``as_bytes()`` returns the
stored address as an ``std::span``.

.. cpp:function:: auto sk::net::make_inet_address(std::uint32_t) -> inet_address

    Create an ``inet_address`` from an ``std::uint32_t`` representing an IP address
    in MSB order.

.. cpp:function:: auto sk::net::make_inet_address(std::string const &) \
                  -> expected<inet_address, std::error_code>

    Create an ``inet_address`` from a literal address string.

INET6 addresses
^^^^^^^^^^^^^^^

An ``inet6_address`` represents an IPv6 address.

.. code-block:: c++

    namespace sk::net {

        struct inet6_family {
            static constexpr address_family_tag tag = /* implementation-defined */;

            static constexpr std::size_t address_size = 128/8;
            struct address_type {
                std::array<std::uint8_t, address_size> bytes;
            };
        };

        template <>
        class address<inet6_family> {
            using address_family = inet6_family;
            using address_type = address_family::address_type;

            auto value() noexcept -> address_type &
            auto value() const noexcept -> address_type const &
            auto as_bytes() const noexcept
                -> std::span<std::byte const, inet6_family::address_size>
        };

    }

A default-constructed ``inet_address`` stores the zero address (``::``).

``value()`` returns the stored address as an array of bytes.  ``as_bytes`` returns the
stored address as an ``std::span``.

.. cpp:function:: auto make_inet6_address(in6_addr) -> inet_address

    Create an ``inet6_address`` from an ``in6_addr``.

.. cpp:function:: auto make_inet6_address(std::string const &) \
                  -> expected<inet6_address, std::error_code>

    Create an ``inet6_address`` from a literal address string.

UNIX addresses
^^^^^^^^^^^^^^

A ``unix_address`` represents a UNIX socket address.

.. code-block:: c++

    namespace sk::net {

        struct unix_family {
            static constexpr address_family_tag tag = /* implementation-defined */;

            static constexpr std::size_t address_size = /* implementation-defined */;
            struct address_type {
                std::array<char, address_size> path;
            };
        };

        template <>
        class address<unix_family> {
            using address_family = unix_family;
            using address_type = address_family::address_type;

            auto value() noexcept -> address_type &
            auto value() const noexcept -> address_type const &
            auto as_bytes() const noexcept
                -> std::span<std::byte const>
        };

    }

A default-constructed ``unix_address`` stores an empty path, which is not a valid address
and cannot be connected to or bound to.

``value()`` returns the stored address as an array.  This array is always the maximum
possible length; if the stored path is shorter than the maximum, it will be NUL-terminated,
otherwise there will be no NUL character.

``as_bytes()`` returns the stored address as a variable-length ``std::span``.  The span
is equal to the length of the stored path and will never contain a NUL character.

.. cpp:function:: auto sk::net::make_unix_address(std::string const &) \
                  -> expected<std::string, std::error_code>

    Create a ``unix_address`` from a string path.

.. cpp:function:: auto sk::net::make_unix_address(std::filesystem::path const &) \
                  -> expected<unix_address, std::error_code>

    Create a ``unix_address`` from a filesystem path.

The unspecified address
^^^^^^^^^^^^^^^^^^^^^^^

An ``address<>`` (also spelled as ``unspecified_address``) represents an address that
could be an IPv4 address, an IPv6 address or a UNIX socket.  ``address<>`` can be queried
at runtime for the type of address it holds, converted to other address types using
``address_cast<>``, or used directly to construct an endpoint.

.. code-block:: c++

    namespace sk::net {

        struct unspecified_family {
            static constexpr address_family_tag tag = /* implementation-defined */;

            static constexpr std::size_t address_size = /* implementation-defined */;
            using address_type = /* implementation-defined */;
        };

        template <>
        class address<unspecified_family> {
            using address_family = unspecified_family;
            using address_type = address_family::address_type;
        };

    }

A default-constructed ``address<>`` stores an undefined value.

.. cpp:function:: template<> auto make_address<unspecified_family>(std::string const &) \
                  -> expected<unspecified_address, std::error_code>

    Create an ``address<>`` from a string, which should be either an INET or INET6
    address literal.  Creating UNIX paths with ``make_address()`` is not supported.

Zero addresses
--------------

The INET and INET6 families support the concept of a zero address, which is
``0.0.0.0`` or ``::``.  The value of a default-constructed address is the zero address,
and a zero address constant is also available as a static class member:

.. code-block:: c++

    inet6_address addr; // str(addr) == "::"
    auto addr2 = inet6_address::zero_address; // str(addr) == "::"
    addr == addr2; // true

To create a zero address for an ``address<>`` at runtime, use
``make_unspecified_zero_address``.

.. cpp:function:: auto make_unspecified_zero_address(address_family_tag af) \
        -> expected<unspecified_address, std::error_code>

    Create an unspecified zero address for the given address family.  For example,
    ``make_unspecified_zero_address(inet6_family::tag)``.

Converting addresses
--------------------

Addresses can be converted between concrete address types and ``address<>``
using ``address_cast``:

.. cpp:function:: template <typename To, typename From> \
                 auto sk::net::address_cast(From &&from)

    Convert an address from the type ``From`` to the type ``To``.

Converting an address type to ``address<>`` always succeeds, unless ``address<>`` cannot
store the given address type, in which case an error is generated at compile-time.

.. code-block:: c++

    inet6_address addr;
    address<> uaddr = address_cast<address<>>(addr); // Cannot fail

Converting an ``address<>`` to an address type may fail at runtime, depending on
whether the ``address<>`` holds the requested address type.

.. code-block:: c++

    address<> uaddr;
    auto addr = address_cast<inet6_address>(uaddr);
    if (addr)
        std::cout << *addr; // Conversion succeeded
    else
        std::cout << addr.error().message(); // Conversion failed.

Resolving addresses
-------------------

Resolving symbolic hostnames to addresses is done with a *resolver* type.  Currently
only one resolver is provided, ``sk::net::system_resolver<>``, which uses the operating
system's resolver library.

.. cpp:class:: template<address_family af = unspecified_family> \
               system_resolver

    Resolve names using a system-specific resolver such as ``getaddrinfo()``.  Since
    most systems do not provide true asynchronous resolvers, this requires spawning a
    new thread to run the name resolution.

    If ``system_resolver`` is instantiated over ``unspecified_family``, it will return
    both INET and INET6 addresses.  If instantiated over ``inet_family`` or ``inet6_family``,
    it will only return addresses for that address family.  No other address families
    are supported.

    ``system_resolver`` does not allocate any memory on the heap and cannot throw
    exceptions.  However, the system resolver functions usually requires a heap
    allocation.

    .. cpp:function:: auto async_resolve(std::optional<std::string> const &name = {}, \
                                         std::optional<std::string> const &service = {})\
                        const noexcept \
                        -> task<expected<__implementation_defined, std::error_code>>

        Resolve the given ``name`` and ``service`` and return the results as an
        implementation-defined container type, which can be forwarded-iterated over to
        obtain the addresses.  The container will contain values of type ``address<af>``.
        When resolving addresses, the ``service`` parameter has no effect and may be omitted.
        If ``name`` is not specified, the zero address will be returned.

    .. cpp:function:: template <std::output_iterator<address<af>> Iterator> \
        auto async_resolve(Iterator &&it, std::optional<std::string> const &name = {}, \
                           std::optional<std::string> const &service = {}) \
                           const noexcept \
            -> task<expected<void, std::error_code>>

        Call ``async_resolve(name, service)`` and copy the result into the given output iterator.

Example
^^^^^^^

.. code-block:: c++

    sk::net::system_resolver<> res;

    auto ret = co_await res.async_resolve(name);
    if (ret)
        std::ranges::copy(*ret, std::ostream_iterator<address<>>(std::cout, "\n"));
    else
        std::cout << ret.error().message() << '\n';


Endpoints
---------

Connecting to a network resource, or binding a channel to accept incoming connections,
requires an *endpoint*, which is a combination of an address (possibly the zero address)
and optionally some protocol-specific additional data.  For INET and INET6 channels,
this is the TCP or UDP port number.  UNIX endpoints do not have any additional data.

TCP endpoints
^^^^^^^^^^^^^

Defined in ``<sk/net/tcpchannel.hxx>``.

.. cpp:class:: tcp_endpoint

    Represents an INET or INET6 address and TCP port number.

    .. cpp:type:: port_type = std::uint16_t

    .. cpp:type:: address_type = address<>

    .. cpp:type:: const_address_type = address<> const

    .. cpp:function:: auto address() const noexcept -> const_address_type &

    .. cpp:function:: auto address() noexcept -> address_type &

        Return the endpoint's address.

    .. cpp:function:: auto port() const noexcept -> port_type

        Return the endpoint's port.

    .. cpp:function:: auto port(port_type p) noexcept -> port_type

        Change the endpoint's port.  Returns the old port.

    .. cpp:function:: auto as_sockaddr_storage() const noexcept -> sockaddr_storage

        Return a ``sockaddr_storage`` structure representing the endpoint's address
        and port.

.. cpp:function:: bool operator==(tcp_endpoint const &a, tcp_endpoint const &b) noexcept

    Compare two ``tcp_endpoint`` for equality.

.. cpp:function:: bool operator<(tcp_endpoint const &a, tcp_endpoint const &b) noexcept

    Compare two ``tcp_endpoint`` for ordering.

.. cpp:function:: auto str(tcp_endpoint const &ep) -> std::string

    Return a string representation of the endpoint in the canonical form.  For INET
    endpoints this is ``127.0.0.1:80``; for INET6 this is ``[::1]:80``.

.. cpp:function:: template<address_family af> \
                  auto make_tcp_endpoint(address<af> const &addr, \
                                  tcp_endpoint::port_type port) noexcept

    Create a TCP endpoint from an address and a port number.  The address family must
    be ``inet_family``, ``inet6_family`` or ``unspecified_family``.

.. cpp:function:: auto make_tcp_endpoint(std::string const &str, \
                                  tcp_endpoint::port_type port) noexcept

    Create a TCP endpoint from an address literal and a port number.

UNIX endpoints
^^^^^^^^^^^^^^

Defined in ``<sk/net/unixchannel.hxx>``.

.. cpp:class:: unix_endpoint

    Represents a UNIX socket endpoint.

    .. cpp:type:: address_type = unix_address

    .. cpp:type:: const_address_type = unix_address const

    .. cpp:function:: auto address() const noexcept -> const_address_type &

    .. cpp:function:: auto address() noexcept -> address_type &

        Return the endpoint's address.

    .. cpp:function:: auto as_sockaddr_storage() const noexcept -> sockaddr_storage

        Return a ``sockaddr_storage`` structure representing the endpoint's address.

.. cpp:function:: bool operator==(unix_endpoint const &a, unix_endpoint const &b) noexcept

    Compare two ``unix_endpoint`` for equality.

.. cpp:function:: bool operator<(unix_endpoint const &a, unix_endpoint const &b) noexcept

    Compare two ``unix_endpoint`` for ordering.

.. cpp:function:: auto str(unix_endpoint const &ep) -> std::string

    Return the endpoint's path as a string.

.. cpp:function:: auto make_unix_endpoint(unix_address const &addr) noexcept \
        -> expected<unix_endpoint, std::error_code>;

    Create a UNIX endpoint from a UNIX address.

.. cpp:function:: auto make_unix_endpoint(address<> const &addr) noexcept \
        -> expected<unix_endpoint, std::error_code>;

    Create a UNIX endpoint from an ``address<>`` which holds a UNIX address.

.. cpp:function:: auto make_unix_endpoint(std::filesystem::path const &addr) noexcept \
        -> expected<unix_endpoint, std::error_code>;

.. cpp:function:: auto make_unix_endpoint(std::string const &addr) noexcept \
        -> expected<unix_endpoint, std::error_code>;

    Create a UNIX endpoint from a filesystem path.

Resolving endpoints
-------------------

To resolve endpoints, use ``tcp_endpoint_system_resolver``.  This has the same interface as
``system_resolver``, except it return ``tcp_endpoint`` objects.  Note that while the
``service`` parameter to ``async_resolve()`` has no effect when resolving addresses, when
resolving endpoints, it will be used to determine the endpoint's port number.  To create
a listening endpoint for all addresses on the local system, use ``async_resolve({}, "service-name")``.

Example
^^^^^^^

.. code-block:: c++

    sk::net::tcp_endpoint_system_resolver res;

    auto ret = co_await res.async_resolve("localhost", "http");
    if (ret)
        std::ranges::copy(*ret, std::ostream_iterator<tcp_endpoint>(std::cout, "\n"));
    else
        std::cout << ret.error().message() << '\n';
