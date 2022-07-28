#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include <chrono>

#include <errno.h>

#include <bit> // endian

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <netdb.h>
#include <arpa/inet.h>

#include <unistd.h>

// fix SOCK_NONBLOCK for e.g. macOS
#ifndef SOCK_NONBLOCK
#include <fcntl.h>
#define SOCK_NONBLOCK O_NONBLOCK
#endif

// dummy SOCK_CLOEXEC for macOS
#ifndef SOCK_CLOEXEC
#define SOCK_CLOEXEC 0
#endif

// signals
#include <sys/signal.h>

#include <algorithm>
#include <array>

#include <qbind/type.h>

auto operator""_KB( const unsigned long long x ){ return 1024*x; }
auto operator""_MB( const unsigned long long x ){ return 1024*1024*x; }
auto operator""_GB( const unsigned long long x ){ return 1024*1024*1024*x; }

#include <iomanip>

// c.java has a write mechanism which boils down to
//   void w(byte x){
//     wBuff[wBuffPos++]=x;
//   }
// You then write to the socket using outStream.write(wBuff)

void throw_errno(const std::string& file, int line, const std::string& function)
{
    auto errno_orig = errno;
    size_t size = 1024;
    char *s = static_cast<char*>(malloc(1024));
    // Not strictly out of memory - malloc can fail in a few ways.
    // Either way at this point we have failed and should be throwing something.
    if (s == nullptr)
        throw std::bad_alloc();

    std::ostringstream ss;
    ss << file << " line " << line << " in " << function << ": ";

    int rc = strerror_r(errno_orig, s, size);
    while (rc != 0)
    {
        // < glibc 2.13. -1 returned (rc) and errno is set.
        if (rc == -1)
            rc = errno;
        
        if (rc == EINVAL)
            throw std::system_error({errno_orig, std::system_category()}, ss.str() + "Failed to get error. Bad errno given to strerror_r.");
        else if (rc == ERANGE)
        {
            size *= 2;
            s = static_cast<char*>(realloc(s, size));
            if (s == nullptr)
            {
                free(s);
                throw std::bad_alloc();
            }
        }
        else if (rc != 0)
            throw std::system_error({rc, std::system_category()}, ss.str() + ": Failed to get error. Unknown strerror_r return code.");

        rc = strerror_r(errno_orig, s, size);
    }
    throw std::system_error({errno_orig, std::system_category()}, ss.str() + s);
}

#define THROW_ERRNO_IF(x)                               \
    if (x)                                              \
        throw_errno(__FILE__, __LINE__, __FUNCTION__);

/**
 * @brief Convert value to native endian
 * 
 * https://mklimenko.github.io/english/2018/08/22/robust-endian-swap/
 * 
 * @tparam T Type to convert
 * @param t Value to convert
 * @param val_endian Endian of current representation.
 * @return std::enable_if_t<std::is_arithmetic_v<T>, T> Value in native endianness.
 */
template<typename T>
typename std::enable_if_t<std::is_arithmetic_v<T>, T>
to_native_endian( T t, std::endian val_endian) noexcept
{
    if (val_endian == std::endian::native || sizeof(T) == 1)
        return std::forward<T>(t);
    union U {
        T val;
        std::array<uint8_t, sizeof(T)> raw;
    } src, dst;

    src.val = t;
    std::reverse_copy(src.raw.begin(), src.raw.end(), dst.raw.begin());
    return dst.val;
}

// TODO: Get all addresses for local and first for remote.
// Make right version of TCP or DOMAIN socket given an address, along with the right options (no delay, reuse socket?)
// Perform a handshake
// SSL handshake (OpenSSL)?
// Serialisation layer
// Emscripten support

class Buffer
{
public:
    struct Deleter
    {
        void operator()(uint8_t* ptr) {
            if(ptr)
                free(ptr);
        }
    };

    Buffer(uint8_t* ptr, size_t size, std::endian endianness = std::endian::native)
    : m_pointer(std::unique_ptr<uint8_t, Deleter>(ptr))
    , m_size(size)
    , m_endianness(endianness)
    {}

    uint8_t* get() const
    {
        return m_pointer.get();
    }

    size_t size() const
    {
        return m_size;
    }

    std::endian endianness() const
    {
        return m_endianness;
    }

    void set_endianness(std::endian e)
    {
        m_endianness = e;
    }

    /**
     * @brief Read from a buffer and get result in machine native endianness
     * 
     * @tparam T Type to read
     * @param current_idx Where to read from. Advanced on read (no bounds checks done)
     * @return std::enable_if_t<std::is_arithmetic_v<T>, T> 
     */
    template<class T>
    typename std::enable_if_t<std::is_arithmetic_v<T>, T>
    read(size_t& current_idx) const noexcept
    {
        T res = *static_cast<const T *>(m_pointer.get() + current_idx);
        current_idx += sizeof(T);
        return to_native_endian(res, m_endianness);
    }

    /**
     * @brief Like read but doesn't change current_idx
     */
    template<class T>
    typename std::enable_if_t<std::is_arithmetic_v<T>, T>
    read_at(size_t current_idx) const noexcept
    {
        return to_native_endian(*static_cast<const T *>(m_pointer.get() + current_idx), m_endianness);
    }

    /**
     * @brief Write to buffer in buffer specified endianness.
     * 
     * @tparam T Type to write
     * @param value Value to write
     * @param current_idx Index to write at (no bounds checks done)
     * @return std::enable_if_t<std::is_arithmetic_v<T>, T> 
     */
    template<class T>
    typename std::enable_if_t<std::is_arithmetic_v<T>, void>
    write(T value, size_t& current_idx)
    {
        value = to_native_endian(value, m_endianness);
        std::memcpy(m_pointer.get() + current_idx, &value, sizeof(T));
        current_idx += sizeof(T);
    }

    /**
     * @brief Like write but doesn't change current_idx
     */
    template<class T>
    typename std::enable_if_t<std::is_arithmetic_v<T>, void>
    write_at(T value, size_t current_idx)
    {
        value = to_native_endian(value, m_endianness);
        std::memcpy(m_pointer.get() + current_idx, &value, sizeof(T));
    }  

private:
    std::unique_ptr<uint8_t, Deleter> m_pointer;
    size_t m_size;
    std::endian m_endianness;
};

class Socket
{
private:
    // https://www.binarytides.com/hostname-to-ip-address-c-sockets-linux/
    static std::vector<std::string> hostname_to_ips(const std::string& hostname)
    {
        struct addrinfo hints, *servinfo, *p;
        memset(&hints, 0, sizeof hints);

        // AF_UNSPEC: Returns socket addresses for any address family (either IPv4
        // or IPv6, for example) that can be used with node and service.
        // Alternative force using AF_INET or AF_INET6.
        hints.ai_family = AF_UNSPEC;
        // TCP socket
        hints.ai_socktype = SOCK_STREAM;

        // Using nullptr so that the port number of the returned address will be uninitialized.
        if (int rc = getaddrinfo(hostname.c_str(), nullptr, &hints, &servinfo); rc != 0)
        {
            throw std::runtime_error(gai_strerror(rc));
        }

        std::vector<std::string> ips;
        for (p = servinfo; p != nullptr; p = p->ai_next)
        {
            switch (p->ai_addr->sa_family)
            {
            case AF_INET:
            {
                auto addr = reinterpret_cast<struct sockaddr_in *>(p->ai_addr);
                char ip_addr[INET_ADDRSTRLEN];
                THROW_ERRNO_IF(inet_ntop(AF_INET, &addr->sin_addr, ip_addr, INET_ADDRSTRLEN) == nullptr);
                ips.emplace_back(ip_addr);
                break;
            }
            case AF_INET6:
            {
                auto addr = reinterpret_cast<struct sockaddr_in6 *>(p->ai_addr);
                char ip_addr[INET6_ADDRSTRLEN];
                THROW_ERRNO_IF(inet_ntop(AF_INET6, &addr->sin6_addr, ip_addr, INET6_ADDRSTRLEN) == nullptr);
                ips.emplace_back(ip_addr);
                break;
            }
            default:
                throw std::runtime_error("Unknown address family: " + std::to_string(p->ai_addr->sa_family));
            };
        }
        freeaddrinfo(servinfo); // all done with this structure
        if (ips.empty())
            throw std::runtime_error("Failed to find address for " + hostname);
        return ips;
    }

    static std::string gethostname()
    {
        // Hostnames are limited to 255 characters (plus a null termination byte).
        char buf[256];
        THROW_ERRNO_IF(::gethostname(buf, 256) != 0);
        return buf;
    }

    int m_fd;
    bool m_is_unix_domain_socket;

public:
    /**
     * @brief Construct a new Socket object
     * 
     * @param hostname : hostname to connect to
     * @param port : port on host to connect to
     * @param credentials : credentials of the form "username:password"
     * @param timeout : Use 0 for no timeout
     * @param use_tls : Use TLS connection
     */
    Socket(const std::string& hostname, 
               uint16_t port, 
               std::chrono::microseconds timeout = {}, // TODO:
               bool use_tls = false) // TODO:
    {
        static const auto local_ips = hostname_to_ips(gethostname());
        const auto ip = hostname_to_ips(hostname).front();
        // Domain
        // If ip of requested hostname is local => AF_UNIX = AF_LOCAL
        //                                ipv4  => AF_INET
        //                                ipv6  => AF_INET6
        // Look for : as IPv4-mapped IPv6 addresses can be like ::FFFF:204.152.189.116
        auto domain = std::find(local_ips.begin(), local_ips.end(), ip) != local_ips.end() ? AF_UNIX :
                      ip.find(':') != std::string::npos ? AF_INET6 : AF_INET;
        // Type
        // SOCK_STREAM = TCP
        // Protocol
        // No protocol number defined for KX IPC
        //  | SOCK_NONBLOCK | SOCK_CLOEXEC
        int fd = socket(domain, SOCK_STREAM, 0);
        THROW_ERRNO_IF(fd == -1);

        // Set tcp nodelay and keep alive like java.c
        const int opt_true = 1;
        THROW_ERRNO_IF(setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &opt_true, sizeof(opt_true)) == -1);
        if (domain != AF_UNIX)
            THROW_ERRNO_IF(setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &opt_true, sizeof(opt_true)) == -1);
        if (0 < timeout.count())
        {
            struct timeval tv
            {
                .tv_sec = static_cast<int>(timeout.count() / 100000),
                .tv_usec = static_cast<int>(timeout.count() % 100000)
            };
            THROW_ERRNO_IF(setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == -1);
            THROW_ERRNO_IF(setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) == -1);
        }

        // Connect
        switch (domain)
        {
            // TODO: Do memset on addrs and then set fields explicitly as recommended by book (listing 57-1)
            case AF_INET:
            {
                struct sockaddr_in addr {
                    .sin_family = AF_INET,
                    .sin_port = htons(port)
                };
                auto rc = inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);
                if (rc == 0)
                    throw std::runtime_error("Invalid IPv4 address");
                else THROW_ERRNO_IF(rc == -1); 

                THROW_ERRNO_IF(connect(fd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) == -1);
                break;
            }
            case AF_INET6:
            {
                struct sockaddr_in6 addr {
                    .sin6_family = AF_INET6,
                    .sin6_port = htons(port)
                };
                auto rc = inet_pton(AF_INET6, ip.c_str(), &addr.sin6_addr);
                if (rc == 0)
                    throw std::runtime_error("Invalid IPv6 address");
                else THROW_ERRNO_IF(rc == -1); 

                THROW_ERRNO_IF(connect(fd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) == -1);
                break;                
            }
            case AF_UNIX:
            {
                const auto path = "/tmp/kx." + std::to_string(port);
                struct sockaddr_un addr {
                    .sun_family = AF_UNIX,
                };
                strcpy(addr.sun_path, path.c_str());
                THROW_ERRNO_IF(connect(fd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) == -1);
                break; 
            }
            default:
                throw std::runtime_error("Unknown address family: " + std::to_string(domain));
        }

        m_fd = fd;
        m_is_unix_domain_socket = domain == AF_UNIX;
    }

    // No copy
    Socket(const Socket &other) = delete;
    Socket &operator=(const Socket &other) = delete;

    // Move
    Socket(Socket&& other) noexcept
    :m_fd(other.m_fd)
    {
        other.m_fd = -1;
    }

    Socket& operator=(Socket&& other) noexcept
    {
        if (this != &other)
        {
            std::swap(m_fd, other.m_fd);
        }
        return *this;
    }

    ~Socket()
    {
        if (m_fd != -1)
        {
            close(m_fd);
            m_fd = -1;
        }
        // TODO: (UDS) When the socket is no longer required, its pathname entry can (and generally
        // should) be removed using unlink() (or remove()). page 1167. - ONLY IF SERVER??
    }

    size_t send(const uint8_t* ptr, size_t size, int flags = 0) const
    {
        // May want to switch to using per message flags. i.e. drop SOCK_NONBLOCK for
        // MSG_DONTWAIT (since Linux 2.2) which gives the same behaviour, but can
        // optionally be enable (otherwise sends which might cause blocking fail)
        auto sent_bytes = ::send(m_fd, ptr, size, flags);
        THROW_ERRNO_IF(sent_bytes == -1);
        return sent_bytes;
    }

    Buffer peek(size_t size) const
    {
        void *buf = malloc(size);
        if (buf == nullptr)
            throw std::bad_alloc();
        // Can use MSG_DONTWAIT here too.
        THROW_ERRNO_IF(::recv(m_fd, buf, size, MSG_PEEK) == -1);
        return {static_cast<uint8_t*>(buf), size};
    }

    // Should be able to make recv wait for expected number of bytes
    Buffer recv(size_t size, int flags = 0) const
    {
        void *buf = malloc(size);
        if (buf == nullptr)
            throw std::bad_alloc();
        // Can use MSG_DONTWAIT here too.
        auto received_bytes = ::recv(m_fd, buf, size, flags);
        THROW_ERRNO_IF(received_bytes== -1);
        if (received_bytes < size)
        {
            buf = realloc(buf, received_bytes);
            if (buf == nullptr)
            {
                free(buf);
                throw std::bad_alloc();
            }
        }
        return {static_cast<uint8_t *>(buf), static_cast<size_t>(received_bytes)};
    }

    bool is_unix_domain_socket() const
    {
        return m_is_unix_domain_socket;
    }
};

enum class MessageType : uint8_t
{
    Async = 0,
    Sync = 1,
    Response = 2
};

/**
 * @brief Implementation of the KX IPC protocol.
 *
 * KX IPC Protocol:
 * 
 * All messages start with a 4 byte header.
 * Index: 0          1       2          3
 * Desc:  endianness msgType compressed excessSize
 * 
 * endianness: 1 if little endian.
 * msgType: 0 - async, 1 - sync, 2 - respose.
 * compressed: (protocol 1+) 1 if compressed, 2 if compressed and original size 8 bytes
 * excessSize: (protocol 5+) (4GB * this) + residual size is total message size
 * 
 * After that is the residual length of the message - including the header and the bytes used to store the size.
 * This is a uint32_t. Anything 4GB + must increment excessSize in the header.
 * 
 * The rest of the message (the payload) is either compressed or uncompressed.
 * 
 * Uncompressed payload:
 * The uncompressed payload can be unpacked recursively.
 * The first byte is always the type.
 * ATOMS: For atoms the next sizeof(type) bytes are the value.
 * FUNCTIONS:
 *      LAMBDA: There is a string (ending in a 0 byte). This is discarded. Then restart the process again.
 *      t<104 (Unary primitive/operator/iterator/projection):
 *          Theres a byte. If its 0 and t == 101 return null, else return "func".
 *      t>105: Restart process again.
 *      PROJECTION or COMPOSITION (t=104 or 105): There is a length TODO: may just be an integer, could be integer or long.
 *          Then perform the serialization operation for the number of times indicated.
 * DICTIONARY: (De)serialize keys then values.
 * Attributes byte (s - 1, u - 2, p - 3, g - 4, largeArray - 128 (protocol 6 only, can be or-ed with attributes))
 * TABLE: Serialise as dictionary (i.e. 99, cols, column values)
 * ARRAY: Then the length of the array as unsigned int (8 bytes if largeArray, else 4 bytes). Then serialise all the entries.
 *      Note: Symbols are null (0) terminated strings. (255 * 4GB) + 4GB (bar 1 byte) is essentially the 1TB limit.
 * 
 * TODO: Why is type 127 a sorted dictionary? https://code.kx.com/q/kb/serialization/#sorted-keyed-table
 * A v3 explanation seems to be here http://jnordwick.github.io/k/kenc.html
 * 
 * Compressed payload:
 * The compressed payload starts with the length the message if it wasn't compressed (including the header).
 * The size is written as 4 bytes (compression level 1) if it will fit, else 8 bytes (compression level 2)
 * 
 * The algorithm doesn't have an identifiable name but it is made up of blocks.
 * A block starts with a single byte header. The size of the block is 1 + 2*popcount(header) + (8-popcount(header)),
 * i.e. there are two bytes in the block for each 1 bit in the header, and one byte for each 0 bit.
 * 
 * Definition: xor-offset: The XOR of two adjacent bytes.
 * 
 * The algorithm effectively tries to compress common sequences of bytes (up to a window size of 255).
 * It starts by check the xor-offset of where you currently are in the buffer against the next byte.
 *      - If that offset hasn't occurred before you cache the index of the offset, and write the
 *        current byte value to the output buffer. (0 in header)
 *      - If that offset has occurred but the first byte is different, update the cache to say the
 *        offset last occurred here. Write the current byte value to the output buffer. (0 in header)
 *      - If that offset has occurred before and the value of the byte where it occurred is the same
 *        (this means you have two runs of two identical bytes) then:
 *          1. Update the cache to say the offset last occurred here.
 *          2. Determine how long the run goes on for (up to 255 or until you hit the end of the buffer)
 *          3. Store the offset (1 byte), run length (1 byte)
 *          4. Make sure there is a 1 in the buffer.
 * Once the final header of the block is determined, go back to the START of the block and store it.
 * 
 * After compression the total length of the message (which is stored after the header) must be updated
 * to reflect the compressed size. This means you cannot stream the serialization/compression, and hence
 * there are protocol levels which limit messages to 2GB or less. 
 *
 * @tparam TSocket : Socket type. Must conform to an interface.
 */
template<class TSocket>
class SocketConnection
{
    // endianness support required.
    // ensure platform is big or little endian - not edge case platform as defined here:
    // https://en.cppreference.com/w/cpp/types/endian
    static_assert(
        (std::endian::little == std::endian::native) != (std::endian::big == std::endian::native),
        "Implementation requires machine is strictly big or little endian.");

private:
    TSocket m_conn;
    uint8_t m_level;

    /**
     * @brief Compress the payload buffer.
     * 
     * Original: https://github.com/KxSystems/javakdb/blob/master/src/main/java/kx/c.java#L500
     * 
     * @param in: Payload buffer (not header/whole message length).
     * @return Buffer: if compression successful return compressed buffer, else original buffer.
     */
    Buffer compress(Buffer in_buff) const
    {
        const size_t in_length = in_buff.size(); // origSize
        uint8_t *in = in_buff.get();

        // compression is only used if compressed data is less than half the size of the original message.
        // 8 + in_length = sizeof(header) + sizeof(message length) + uncompressed payload length
        const size_t out_length = (8 + in_length) / 2; // e
        uint8_t *out = static_cast<uint8_t *>(malloc(out_length)); // wBuff
        if (out == nullptr)
            throw std::bad_alloc();

        // Setup compression variables
        // i: Mask to turn on bits in the header.
        uint8_t block_header_mask = 0;
        // f: Current block header
        uint8_t current_block_header = 0;

        // s/s0: Index on input buffer on current/previous iteration.
        size_t in_idx = 0; //    = header_size + buf_size_size;
        size_t in_idx_prev = 0;

        // h/h0: xor_offset between in_idx and in_idx+1
        uint8_t xor_offset, xor_offset_prev;

        // c: Index where current block header will go when done. After header, compressed size, full size.
        size_t header_idx = 0; //   = header_size + (2 * buf_size_size);

        // d: start point to start writing from
        size_t out_idx = header_idx;

        // a: Cache where offset=index occurred. (0 initialised => offset hasn't occurred yet)
        size_t xor_offset_cache[256];
        // p: Last time current offset occurred
        size_t xor_offset_prev_idx = 0;
        // g: Whether to invalidate cache (true) or check for a run (false)
        bool invalidate_cache = false;

        // r: Run start index
        size_t run_start_idx = 0;
        // q: Final index to look to when searching for runs.
        size_t look_until_idx = 0; 

        // t from c.java unused -> same as in_length

        for (; in_idx < in_length; block_header_mask *= 2)
        {
            // Start a new block: overflow allows this to be activated each 8 iterations
            if (block_header_mask == 0)
            {
                // if you're within 17 bytes then return original buffer as wont be < 1/2 original size.
                // Technically this could be 9 as a block can be as small as 9 bytes, but this saves
                // always checking indexes as you know you're clear if you have 17 bytes still free.
                if (out_length-17 < out_idx)
                    return in_buff;

                block_header_mask = {1};
                out[header_idx] = current_block_header; // store old block header.
                header_idx = out_idx++; // TODO: Check: This is assignment then increment.
                current_block_header = {0};
            }

            invalidate_cache=(in_idx>in_length - 3) || // don't save by compressing a run at this point.
                       (0 == ( // index has not occurred before
                           xor_offset_prev_idx=xor_offset_cache[
                               xor_offset=in[in_idx]^in[in_idx+1]
                           ])) ||
                       (in[in_idx] != in[xor_offset_prev_idx]); // different start value for offset

            if(in_idx_prev)
            {
                xor_offset_cache[xor_offset_prev] = in_idx_prev;
                in_idx_prev = 0;
            }

            // if invalidating cache write the new start byte for the offset in to the out buffer
            if (invalidate_cache)
            {
                xor_offset_prev = xor_offset;
                in_idx_prev = in_idx;
                out[out_idx++] = in[in_idx++];
            }
            else
            {
                xor_offset_cache[xor_offset] = in_idx;
                current_block_header |= block_header_mask;
                xor_offset_prev_idx += 2; // after the the two start bytes
                run_start_idx = in_idx += 2;
                look_until_idx = std::min(in_idx + 255, in_length);
                // check for the run
                while(in[xor_offset_prev_idx] == in[in_idx] && ++in_idx < look_until_idx)
                    ++xor_offset_prev_idx;

                out[out_idx++] = xor_offset;
                out[out_idx++] = in_idx - run_start_idx;
            }
        }
        out[header_idx] = current_block_header;
        Buffer res{static_cast<uint8_t *>(realloc(out, out_idx)), out_idx, in_buff.endianness()};
        if (res.get() == nullptr)
        {
            // no need to throw as red destructor will do cleanup
            throw std::bad_alloc();
        }
        return res;
    }

    /**
     * @brief Decompress a compressed payload buffer.
     * 
     * Original: https://github.com/KxSystems/javakdb/blob/master/src/main/java/kx/c.java#L562
     * 
     * @param in_buff: Buffer to decompress containing compressed payload only.
     * @param level: 1 if 4 byte size, 2 if 8 byte size
     * @return Buffer: A decompressed message payload
     */
    Buffer decompress(Buffer in_buff, uint8_t level) const
    {
        // TODO: How to read original length from compressed buffer
        const size_t in_length = in_buff.size(); 
        uint8_t *in = in_buff.get();
        // d: Index in input buff. (start after size of payload)
        size_t in_idx = level == 1 ? 4 : 8;

        // read uncompressed length and make appropriate buffer. Remove 8 bytes for header and the bytes to store buffer size.
        const size_t out_length = (level == 1 ? in_buff.read_at<uint32_t>(8) : in_buff.read_at<uint64_t>(8)) - 8 - in_idx;
        // dst: Buffer for decompressed payload
        uint8_t *out = static_cast<uint8_t *>(malloc(out_length)); 
        if (out == nullptr)
            throw std::bad_alloc();
        // s: Current index in output buffer
        size_t out_idx = 0;

        // Setup compression variables
        // i: Mask to turn on bits in the header.
        uint8_t block_header_mask = 0;
        // f: Current block header
        uint8_t current_block_header = 0;

        // a: Cache where offset=index occurred.
        size_t xor_offset_cache[256];
        // p: Index up to which cache has been populated
        size_t cache_valid_to_idx = 0;

        // n: run length after first two bytes in run.
        size_t run_length = 0;
        // r: Index to copy from, from previous occurrence of run.
        size_t run_start_idx = 0;

        while (out_idx < out_length)
        {
            if (block_header_mask == 0)
            {
                current_block_header = in[in_idx++];
                block_header_mask = 1;
            }

            if ((current_block_header&block_header_mask) != 0)
            {
                // get the two bytes which start the run (i.e. from which the offset was obtained)
                // and the rest of the run (stored in input buffer) in to output buffer
                run_start_idx = xor_offset_cache[in[in_idx++]];
                run_length = in[in_idx++];
                std::memcpy(out + out_idx, out + run_start_idx, 2+run_length);
                out_idx += 2;
            }
            else
            {
                // get the new starter byte for the xor_offset
                out[out_idx++] = in[in_idx++];
            }

            while (cache_valid_to_idx < out_idx-1)
                xor_offset_cache[out[cache_valid_to_idx] ^ out[cache_valid_to_idx + 1]] = cache_valid_to_idx++;

            if ((current_block_header&block_header_mask) != 0)
                cache_valid_to_idx = out_idx += run_length;

            block_header_mask *= 2; // will cycle to 0 via overflow
        }

        return {out, out_idx, in_buff.endianness()};
    }

    /**
     * @brief 
     * 
     * @param in_buff : Payload buffer
     * @param msg_type : Message type to send.
     */
    void send_impl(Buffer in_buff, MessageType msg_type) const
    {
        // need to use a buffer as header endianness must match in_buff
        Buffer header{static_cast<uint8_t *>(calloc(16, 1)), 16, in_buff.endianness()};
        if (header.get() == nullptr)
            throw std::bad_alloc(); // no need to throw as red destructor will do cleanup
        size_t header_size = 8; // default header size (can be 8, 12, or 16)
        header.write_at(std::endian::little == in_buff.endianness(), 0);
        header.write_at(static_cast<uint8_t>(msg_type), 1);
        // start out as uncompressed. and no 4GB multiplier
  
        // original in_buff size
        const size_t orig_payload_size = in_buff.size();
        const size_t uncompressed_msg_size = header_size + orig_payload_size;
        if (uncompressed_msg_size >= 2_GB && m_level < 5)
            throw std::runtime_error("Protocol level only supports messages up to 2GB");

        // compression available if 0<level, not local, and total uncompressed message would be more than 2000 bytes.
        // try to compress if we should. Update header to reflect outcome.
        if(0 < m_level && !m_conn.is_unix_domain_socket() && 2000 < uncompressed_msg_size)
        {
            in_buff = compress(std::move(in_buff));
            // if in_buff not smaller then compression skipped. Write uncompressed message size
            if (in_buff.size() < orig_payload_size)
            {
                header.write_at<uint8_t>(uncompressed_msg_size < 4_GB ? 1 : 2, 2);
                if (uncompressed_msg_size < 4_GB)
                    header.write<uint32_t>(uncompressed_msg_size, header_size);
                else
                    header.write<uint64_t>(uncompressed_msg_size, header_size);
            }
        }

        // write full message size
        const size_t final_msg_size = header_size + in_buff.size();
        header.write_at(static_cast<uint8_t>(final_msg_size / 4_GB), 3);
        header.write_at(static_cast<uint32_t>(final_msg_size % 4_GB), 4);

        // more flag would be helpful here
        m_conn.send(header.get(), header_size);
        m_conn.send(in_buff.get(), in_buff.size());
    }

public:
    /**
     * @brief Construct a new Socket Connection object
     * 
     * @param conn : Connection to communicate protocol over.
     * @param credentials :  String of the form "username:password"
     * @param level : Protocol level.
     * 
     * Protocol level: https://code.kx.com/q/basics/ipc/#handshake.
     *  0   : (V2.5) no compression, no timestamp, no timespan, no UUID
     *  1..2: (V2.6-2.8) compression, timestamp, timespan
     *  3   : (V3.0) compression, timestamp, timespan, UUID
     *  4   : reserved
     *  5   : support msgs >2GB; vectors must each have a count ≤ 2 billion
     *  6   : support msgs >2GB and vectors may each have a count > 2 billion
     * 
     * Size restrictions are only enforced on serialization.
     * Compression is applied on send according to the protocol level, and on receive according to the inbound message.
     * Type restrictions should be enforced by a serializer.
     */
    SocketConnection(TSocket&& conn, const std::string& credentials = "", uint8_t level = 6)
    :m_conn(std::move(conn))
    ,m_level(level)
    {
        if (level == 4 || 6 < level)
            throw std::domain_error("Protocol level must be less than 7 and not 4.");

        // perform handshake
        // msgmore would be helpful here to do only one send, but not supported on macos.
        m_conn.send(reinterpret_cast<const uint8_t *>(credentials.data()), credentials.size());
        const uint8_t capability_msg[2] = {level, 0};
        m_conn.send(capability_msg, sizeof(capability_msg));

        const auto capability_response = m_conn.recv(1);
        if (capability_response.size() == 0)
            throw std::runtime_error("Access: authentication failed");
        const uint8_t supported_level = *capability_response.get();
        if (supported_level < level)
            throw std::runtime_error("KDB instance supports insufficient protocol level: " + std::to_string(supported_level));
    }

    const TSocket& connection() const noexcept
    {
        return m_conn;
    }

    uint8_t protocol_level() const noexcept
    {
        return m_level;
    }

    Buffer send(Buffer in_buff) const
    {
        send_impl(std::move(in_buff), MessageType::Sync);

        auto [buff, msg_type] = recv();
        if (msg_type != MessageType::Response)
            throw std::runtime_error("Expected response message got " + (msg_type == MessageType::Sync ? "sync" : "async") + " message");
        return buff;
    }

    void send_async(Buffer in_buff) const
    {
        send_impl(std::move(in_buff), MessageType::Async);
    }

    std::pair<Buffer, MessageType> recv() const
    {
        Buffer header = m_conn.recv(8);
        if (header.size() != 8)
            throw std::runtime_error("Could not get full header");

        const auto endianness = header.get()[0] == 1 ? std::endian::little : std::endian::big;
        const MessageType msg_type{header.get()[1]};
        const uint8_t compression_level = header.get()[2];
        // 4GB times index 3 plus the residual uint32_t at index 4, minus 8 byte header gives payload size
        const size_t payload_size = (4_GB * header.read_at<uint8_t>(3)) + header.read_at<uint32_t>(4) - 8;
        auto payload = m_conn.recv(payload_size);
        if (payload.size() != payload_size)
            throw std::runtime_error("Could not get full payload");
        payload.set_endianness(endianness);

        // decompress if required regardless of level
        if (compression_level)
            payload = decompress(std::move(payload), compression_level);

        return {std::move(payload), msg_type};
    }
};

class Serializer
{
private:
    uint8_t m_level;

    void supports_type(qbind::Type t)
    {
        const bool unsupported = (m_level < 3 && t == qbind::Type::GUID) ||
            (m_level == 0 && (t == qbind::Type::Timestamp || t == qbind::Type::Timespan));
        if (unsupported)
        {
            std::ostringstream ss;
            ss << "Protocol level " << std::to_string(m_level) << " does not support " << t;
            throw std::runtime_error(ss.str());
        }
    }

    size_t get_buffer_length_size() const noexcept
    {
        return m_level == 6 ? 8 : 4;
    }

public:

    /**
     * @brief A helper class for (de)serializing to KX IPC format. 
     * 
     * @param level : Protocol level.
     * 
     * Protocol level: https://code.kx.com/q/basics/ipc/#handshake.
     *  0   : (V2.5) no compression, no timestamp, no timespan, no UUID
     *  1..2: (V2.6-2.8) compression, timestamp, timespan
     *  3   : (V3.0) compression, timestamp, timespan, UUID
     *  4   : reserved
     *  5   : support msgs >2GB; vectors must each have a count ≤ 2 billion
     *  6   : support msgs >2GB and vectors may each have a count > 2 billion
     * 
     * Array size and type constraints are applied on serialise
     */
    Serializer(uint8_t level)
    :m_level(level)
    {}

    /**
     * @brief Serialize numeric arrays (everything except symbol)
     * 
     * @tparam Iter 
     * @param start 
     * @param end 
     * @return Buffer 
     */
    template <
        qbind::Type Type,
        typename Iter,
        std::enable_if_t<std::is_arithmetic_v<typename std::iterator_traits<Iter>::value_type>, std::nullptr_t> = nullptr
    >
    Buffer serialize(Iter start, Iter end)
    {
        using v_type = typename qbind::internal::c_type<Type>::value;
        using i_type = typename std::iterator_traits<Iter>::value_type;
        static_assert(std::is_same_v<v_type, i_type>,
                      "Iterator value type does not match underlying C type of KX Type");
        supports_type(Type);

        const size_t length = end - start;
        if (m_level < 6 && length > 2000000000) // 2 billion
            throw std::runtime_error("Only protocol level 6 supports arrays over 2 billion elements");
        
        const bool big_array = length >= 4_GB;
        const size_t buf_length = 2 + (big_array ? 8 : 4) + (length * sizeof(v_type));

        Buffer buf{static_cast<uint8_t *>(malloc(buf_length)), buf_length};
        if (buf.get() == nullptr)
            throw std::bad_alloc();
        // write type and header
        size_t idx = 0;
        buf.write(static_cast<signed char>(Type), idx); // type
        buf.write<uint8_t>(big_array ? 128 : 0, idx); // attributes TODO: implement s, u, p, g
        if (big_array)
            buf.write<int64_t>(length, idx);
        else
            buf.write<int32_t>(length, idx);

        // find the initial point for data and copy across. Can use execution policy here if wanted.
        auto data_base = reinterpret_cast<v_type *>(buf.get()+idx);
        std::copy(start, end, data_base);

        return buf;
    }

    // TODO symbol array

    /**
     * @brief Serialise an atom (everything except symbol)
     * 
     * @tparam Type 
     * @tparam T 
     * @param value 
     * @return Buffer 
     */
    template <
        qbind::Type Type,
        typename T,
        std::enable_if_t<std::is_arithmetic_v<T>, std::nullptr_t> = nullptr
    >
    Buffer serialize(T value)
    {
        using v_type = typename qbind::internal::c_type<Type>::value;
        static_assert(std::is_same_v<T, v_type>,
                      "Iterator value type does not match underlying C type of KX Type");
        supports_type(Type);
        const size_t buf_length = 1 + sizeof(T);

        Buffer buf{static_cast<uint8_t *>(malloc(buf_length)), buf_length};
        if (buf.get() == nullptr)
            throw std::bad_alloc();

        buf.write_at(-static_cast<signed char>(Type), 0);
        buf.write_at<v_type>(value, 1);

        return buf;
    }
};

void print_buffer(Buffer& buf, size_t cnt = 0)
{
    if (cnt == 0)
        cnt = buf.size();

    for (int i = 0; i < cnt; ++i)
        std::cout << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(buf.get()[i]) << " ";
    std::cout << std::endl;
}

int main()
{
    uint8_t level = 6;
    auto s = Socket("127.0.0.1", 5000);
    SocketConnection conn(std::move(s), "", level);

    std::cout << (std::endian::native == std::endian::big) << std::endl;
    std::cout << (std::endian::native == std::endian::little) << std::endl;

    // Buffer buf{static_cast<uint8_t*>(malloc(5)), 5};
    // if (buf.get() == nullptr)
    //     throw std::bad_alloc();

    // buf.write_at<signed char>(-6, 0);
    // buf.write_at<int32_t>(999, 1);

    Serializer ser(level);
    conn.send_async(ser.serialize<qbind::Type::Int>(666));

    std::vector<int32_t> vec{1, 2, 3, 4, 5};
    conn.send_async(ser.serialize<qbind::Type::Int>(vec.begin(), vec.end()));
    //auto b = ser.serialize<qbind::Type::Int>(vec.begin(), vec.end());
    // print_buffer(b);

    // while (true)
    // {
    //     sleep(5);
    //     std::cout << "hello" << std::endl;
    //     auto buf = conn.connection().recv(5000);
    //     if (buf.size() > 0)
    //     {
    //         std::cout << buf.size() << std::endl;
    //         print_buffer(buf, 100);
    //         break;
    //     }

    // }

    // std::string up = "kparrott:pwd";
    // conn.send(reinterpret_cast<uint8_t *>(up.data()), up.size());
    // uint8_t capability[2] = {5, 0};
    // conn.send(capability, 2);

    // // std::string msg = "kparrott:pwd\5\0";
    // // std::cout << msg << msg.size() << std::endl;


    // auto resp = conn.recv(1);
    // std::cout << "Size: " << resp.size() << std::endl;
    // std::cout << std::to_string(*resp.get()) << std::endl;


    // const auto hostname = gethostname();
    // const auto local_ips = hostnames_to_ips(hostname);

    // for (const auto& x: local_ips)
    // {
    //     std::cout << x << std::endl;
    // }



    // After this should be down to send and receive
}



// can we use readv/writev instead of the wait flag
// When SOMAXCONN limit is 128 how do you get to 1024 for kdb, or higher?
// accept can take NULL and 0 to ignore getting remote address

// If connect() fails and we wish to reattempt the connection, then SUSv3 specifies
// that the portable method of doing so is to close the socket, create a new socket, and
// reattempt the connection with the new socket.

// A socket may be closed using the close() system call or as a consequence of the
// application terminating. Afterward, when the peer application attempts to
// read from the other end of the connection, it receives end-of-file (once all buffered data has been read). If the peer application attempts to write to its socket,
// it receives a SIGPIPE signal, and the system call fails with the error EPIPE. As we
// noted in Section 44.2, the usual way of dealing with this possibility is to ignore
// the SIGPIPE signal and find out about the closed connection via the EPIPE error.

// SO_RCVBUF to override incoming TCP buffer. (See page 1192). Check flow control for slow consumer.

// sockaddr_storage if we need to store any type of address. Removes IP version dependency
// (if we need send_to and recv_from)

// I/O models (I/O multiplexing, signal-driven I/O, or epoll) 

// implement readn and writen as on p1255 => MSG_WAITALL on recv does the same as readn except restart if there's a signal.

//// Single base IO:
// A program can set one of the following dispositions for a signal:
// - The default action should occur. This is useful to undo an earlier change of the disposition of the signal to something other than its default.
// - The signal is ignored. This is useful for a signal whose default action would be to terminate the process.
// - A signal handler is executed.

// Notifying the kernel that a handler function should be invoked is usually referred to as installing or establishing a signal handler.

// SIGALRM
// The kernel generates this signal upon the expiration of a real-time timer set by a call to alarm() or setitimer().
// SIGVTALRM
// Like SIGALRM but for a virtual (user mode CPU time) timer.
// SIGIO
// Input has become available.
// SIGPIPE
// Reader end of a pipe/FIFO/socket has been closed.
// SIGQUIT
// Control backslash to get a core dump. Useful for infinite loops.
// SIGTERM
// A well-designed application will have a handler for SIGTERM that causes the application to exit gracefully, 
// cleaning up temporary files and releasing other resources beforehand.

// Normal signals are not queued. There is a bitmask identifying which have been received. If the signal is received
// multiple times it is later received just once. Real time signals are queued though.

// Calling pause() suspends execution of the process until the call is interrupted by a
// signal handler (or until an unhandled signal terminates the process).
//     while(true) pause() ??

// The SUSv3 definition of a reentrant function is one “whose effect, when called
// by two or more threads, is guaranteed to be as if the threads each executed the
// function one after the other in an undefined order, even if the actual execution is interleaved.”

// A function is async-signal-safe either because it
// is reentrant or because it is not interruptible by a signal handler.

// In other words, when writing signal
// handlers, we have two choices:
// - Ensure that the code of the signal handler itself is reentrant and that it calls
// only async-signal-safe functions.
// - Block delivery of signals while executing code in the main program that calls
// unsafe functions or works with global data structures also updated by the signal handler.

// The problem with the second approach is that, in a complex program, it can be difficult to ensure that a signal handler will never interrupt the main program while it
// is calling an unsafe function. For this reason, the above rules are often simplified to
// the statement that we must not call unsafe functions from within a signal handler.

// If we set up the same handler function to deal with several different signals or
// use the SA_NODEFER flag to sigaction(), then a handler may interrupt itself. As a
// consequence, the handler may be nonreentrant if it updates global (or static)
// variables, even if they are not used by the main program.

// An errno switch should be used at the start and end of a signal handler to ensure global state is maintained.

