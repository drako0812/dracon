#pragma once

#include <array>
#include <bitset>
#include <cstdint>
#include <optional>

namespace dracon {

    class UID {
      public:
        UID();
        UID(const std::bitset<120> & data);
        static auto Generate() -> UID;
        static auto Empty() -> UID;
        auto        ToBytes() -> std::array<std::uint8_t, 120 / 8>;
        auto        operator==(const UID & rhs) const -> bool;
        auto        ToString16() const -> std::string;
        auto        ToString32() const -> std::string;
        auto        ToString64() const -> std::string;
        static auto FromString16(const std::string_view buffer) -> std::optional<UID>;
        static auto FromString32(const std::string_view buffer) -> std::optional<UID>;
        static auto FromString64(const std::string_view buffer) -> std::optional<UID>;

      private:
        static auto bitsToByte(const std::bitset<8> & bits) -> std::uint8_t;
        static auto bitsToNibble(const std::bitset<4> & bits) -> std::uint8_t;
        static auto bitsToNibbles(const std::bitset<120> & source, std::array<std::uint8_t, 30> & dest) -> void;
        static auto bitsTo5Bit(const std::bitset<5> & bits) -> std::uint8_t;
        static auto bitsTo5Bits(const std::bitset<120> & source, std::array<std::uint8_t, 24> & dest) -> void;
        static auto bitsTo6Bit(const std::bitset<6> & bits) -> std::uint8_t;
        static auto bitsTo6Bits(const std::bitset<120> & source, std::array<std::uint8_t, 20> & dest) -> void;
        static auto in(char ch, const char * const options) -> std::optional<std::size_t>;
        static auto nibbleToBits(std::uint8_t nibble) -> std::bitset<4>;
        static auto nibblesToBits(const std::array<std::uint8_t, 30> & source, std::bitset<120> & dest) -> void;
        static auto bit5ToBits(std::uint8_t bits) -> std::bitset<5>;
        static auto bits5ToBits(const std::array<std::uint8_t, 24> & source, std::bitset<120> & dest) -> void;
        static auto bit6ToBits(std::uint8_t bits) -> std::bitset<6>;
        static auto bits6ToBits(const std::array<std::uint8_t, 20> & source, std::bitset<120> & dest) -> void;
        static auto rngDataToBits(unsigned int data) -> std::bitset<32>;
        static auto rngBufferToBits(const std::array<unsigned int, 4> & source, std::bitset<120> & dest) -> void;

      private:
        static inline const char CHARS16[] = "0123456789ABCDEF";
        static inline const char CHARS32[] = "0123456789ABCDEFGHIJKLMNOPQRSTUV";
        static inline const char CHARS64[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-_";

        std::bitset<120>         data;
    };

} // namespace dracon
