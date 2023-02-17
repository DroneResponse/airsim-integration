#include <cstdint>
#include <arpa/inet.h>
#include <bit>

inline constexpr int64_t reverse_bytes(const int64_t value) {
    // if we have 0x 22222222 11111111
    auto hb = (int64_t) htonl(value) << 32; // 11111111 00000000
    auto lb = (int64_t) htonl(value >> 32); // 00000000 22222222
    return hb + lb; // 11111111 22222222
}

inline constexpr int64_t htonll(const int64_t value) {
    // This is a "constexpr if statement" If native == little, then the compiler
    // will discarded the false branch otherwise the true branch is discarded.
    if constexpr (std::endian::native == std::endian::little) {
        return reverse_bytes(value);
    }
    else {
        return value;
    }
}

inline constexpr int64_t ntohll(const int64_t value) {
    return htonll(value);
}

// todo: make this a template
inline constexpr uint64_t reverse_bytes(const uint64_t value) {
    // if we have 0x 22222222 11111111
    auto hb = (uint64_t) htonl(value) << 32; // 11111111 00000000
    auto lb = (uint64_t) htonl(value >> 32); // 00000000 22222222
    return hb + lb; // 11111111 22222222
}

inline constexpr uint64_t htonll(const uint64_t value) {
    // This is a "constexpr if statement" If native == little, then the compiler
    // will discarded the false branch otherwise the true branch is discarded.
    if constexpr (std::endian::native == std::endian::little) {
        return reverse_bytes(value);
    }
    else {
        return value;
    }
}

inline constexpr uint64_t ntohll(const uint64_t value) {
    return htonll(value);
}