#include "uid.hpp"
#include <cstring>
#include <optional>
#include <random>
// #include "effolkronium/random.hpp"

namespace dracon {

    // using UIDRandom = effolkronium::basic_random_static<std::random_device>;

    UID::UID() { data.reset(); }
    UID::UID(const std::bitset<120> & data) : data{data} { }
    auto UID::Generate() -> UID {
        auto                        ret = UID();
        std::array<unsigned int, 4> rng;
        static auto                 rd = std::random_device{};
        for (std::size_t i = 0; i < rng.size(); i++) { rng.at(i) = rd(); }
        rngBufferToBits(rng, ret.data);
        return ret;
    }
    auto UID::Empty() -> UID { return UID(); }
    auto UID::ToBytes() -> std::array<std::uint8_t, 120 / 8> {
        std::array<std::uint8_t, 120 / 8> ret{};

        for (std::size_t i = 0; i < (120 / 8); i++) {
            std::bitset<8> bits{};
            bits[0] = data[i * 8];
            bits[1] = data[(i * 8) + 1];
            bits[2] = data[(i * 8) + 2];
            bits[3] = data[(i * 8) + 3];
            bits[4] = data[(i * 8) + 4];
            bits[5] = data[(i * 8) + 5];
            bits[6] = data[(i * 8) + 6];
            bits[7] = data[(i * 8) + 7];
            ret[i]  = UID::bitsToByte(bits);
        }
        return ret;
    }
    auto UID::operator==(const UID & rhs) const -> bool { return data == rhs.data; }
    auto UID::ToString16() const -> std::string {
        std::string buffer{};
        buffer.resize(30);
        std::array<std::uint8_t, 30> out;
        bitsToNibbles(data, out);
        for (std::size_t i = 0; i < 30; i++) { buffer.at(i) = CHARS16[out[i]]; }
        return buffer;
    }
    auto UID::ToString32() const -> std::string {
        std::string buffer{};
        buffer.resize(24);
        std::array<std::uint8_t, 24> out;
        bitsTo5Bits(data, out);
        for (std::size_t i = 0; i < 24; i++) { buffer.at(i) = CHARS32[out[i]]; }
        return buffer;
    }
    auto UID::ToString64() const -> std::string {
        std::string buffer{};
        buffer.resize(20);
        std::array<std::uint8_t, 20> out;
        bitsTo6Bits(data, out);
        for (std::size_t i = 0; i < 20; i++) { buffer.at(i) = CHARS64[out[i]]; }
        return buffer;
    }
    auto UID::FromString16(const std::string_view buffer) -> std::optional<UID> {
        std::array<std::uint8_t, 30> data_parts{};
        for (std::size_t i = 0; i < 30; i++) {
            const auto result = in(buffer.at(i), CHARS16);
            if (result.has_value()) {
                data_parts.at(i) = static_cast<std::uint8_t>(result.value());
            } else {
                return std::nullopt;
            }
        }
        std::bitset<120> data{};
        nibblesToBits(data_parts, data);
        return UID{data};
    }
    auto UID::FromString32(const std::string_view buffer) -> std::optional<UID> {
        std::array<std::uint8_t, 24> data_parts{};
        for (std::size_t i = 0; i < 24; i++) {
            const auto result = in(buffer.at(i), CHARS32);
            if (result.has_value()) {
                data_parts.at(i) = static_cast<std::uint8_t>(result.value());
            } else {
                return std::nullopt;
            }
        }
        std::bitset<120> data{};
        bits5ToBits(data_parts, data);
        return UID{data};
    }
    auto UID::FromString64(const std::string_view buffer) -> std::optional<UID> {
        std::array<std::uint8_t, 20> data_parts{};
        for (std::size_t i = 0; i < 20; i++) {
            const auto result = in(buffer.at(i), CHARS64);
            if (result.has_value()) {
                data_parts.at(i) = static_cast<std::uint8_t>(result.value());
            } else {
                return std::nullopt;
            }
        }
        std::bitset<120> data{};
        bits6ToBits(data_parts, data);
        return UID{data};
    }
    auto UID::bitsToByte(const std::bitset<8> & bits) -> std::uint8_t {
        return static_cast<std::uint8_t>(bits.to_ulong() & 0xFF);
    }
    auto UID::bitsToNibble(const std::bitset<4> & bits) -> std::uint8_t {
        return static_cast<std::uint8_t>(bits.to_ulong() & 0xF);
    }
    auto UID::bitsToNibbles(const std::bitset<120> & source, std::array<std::uint8_t, 30> & dest) -> void {
        for (std::size_t i = 0; i < dest.size(); i++) {
            std::bitset<4> bits{};
            bits[0]    = source[i * 4];
            bits[1]    = source[(i * 4) + 1];
            bits[2]    = source[(i * 4) + 2];
            bits[3]    = source[(i * 4) + 3];
            dest.at(i) = bitsToNibble(bits);
        }
    }
    auto UID::bitsTo5Bit(const std::bitset<5> & bits) -> std::uint8_t {
        return static_cast<std::uint8_t>(bits.to_ulong() & 0b11111);
    }
    auto UID::bitsTo5Bits(const std::bitset<120> & source, std::array<std::uint8_t, 24> & dest) -> void {
        for (std::size_t i = 0; i < dest.size(); i++) {
            std::bitset<5> bits{};
            bits[0]    = source[i * 5];
            bits[1]    = source[(i * 5) + 1];
            bits[2]    = source[(i * 5) + 2];
            bits[3]    = source[(i * 5) + 3];
            bits[4]    = source[(i * 5) + 4];
            dest.at(i) = bitsTo5Bit(bits);
        }
    }
    auto UID::bitsTo6Bit(const std::bitset<6> & bits) -> std::uint8_t {
        return static_cast<std::uint8_t>(bits.to_ulong() & 0b111111);
    }
    auto UID::bitsTo6Bits(const std::bitset<120> & source, std::array<std::uint8_t, 20> & dest) -> void {
        for (std::size_t i = 0; i < dest.size(); i++) {
            std::bitset<6> bits{};
            bits[0]    = source[i * 6];
            bits[1]    = source[(i * 6) + 1];
            bits[2]    = source[(i * 6) + 2];
            bits[3]    = source[(i * 6) + 3];
            bits[4]    = source[(i * 6) + 4];
            bits[5]    = source[(i * 6) + 5];
            dest.at(i) = bitsTo6Bit(bits);
        }
    }
    auto UID::in(char ch, const char * const options) -> std::optional<std::size_t> {
        for (std::size_t i = 0; i < std::strlen(options); i++) {
            if (ch == options[i]) { return i; }
        }
        return std::nullopt;
    }
    auto UID::nibbleToBits(std::uint8_t nibble) -> std::bitset<4> {
        std::bitset<4> bits;
        bits[0] = (nibble >> 3) & 1;
        bits[1] = (nibble >> 2) & 1;
        bits[2] = (nibble >> 1) & 1;
        bits[3] = nibble & 1;
        return bits;
    }
    auto UID::nibblesToBits(const std::array<std::uint8_t, 30> & source, std::bitset<120> & dest) -> void {
        for (std::size_t i = 0; i < source.size(); i++) {
            auto bits         = nibbleToBits(source.at(i));
            dest[i * 4]       = bits[0];
            dest[(i * 4) + 1] = bits[1];
            dest[(i * 4) + 2] = bits[2];
            dest[(i * 4) + 3] = bits[3];
        }
    }
    auto UID::bit5ToBits(std::uint8_t bits) -> std::bitset<5> {
        std::bitset<5> obits;
        obits[0] = (bits >> 4) & 1;
        obits[1] = (bits >> 3) & 1;
        obits[2] = (bits >> 2) & 1;
        obits[3] = (bits >> 1) & 1;
        obits[4] = bits & 1;
        return obits;
    }
    auto UID::bits5ToBits(const std::array<std::uint8_t, 24> & source, std::bitset<120> & dest) -> void {
        for (std::size_t i = 0; i < source.size(); i++) {
            auto bits         = bit5ToBits(source.at(i));
            dest[i * 5]       = bits[0];
            dest[(i * 5) + 1] = bits[1];
            dest[(i * 5) + 2] = bits[2];
            dest[(i * 5) + 3] = bits[3];
            dest[(i * 5) + 4] = bits[4];
        }
    }
    auto UID::bit6ToBits(std::uint8_t bits) -> std::bitset<6> {
        std::bitset<6> obits;
        obits[0] = (bits >> 5) & 1;
        obits[1] = (bits >> 4) & 1;
        obits[2] = (bits >> 3) & 1;
        obits[3] = (bits >> 2) & 1;
        obits[4] = (bits >> 1) & 1;
        obits[5] = bits & 1;
        return obits;
    }
    auto UID::bits6ToBits(const std::array<std::uint8_t, 20> & source, std::bitset<120> & dest) -> void {
        for (std::size_t i = 0; i < source.size(); i++) {
            auto bits         = bit6ToBits(source.at(i));
            dest[i * 6]       = bits[0];
            dest[(i * 6) + 1] = bits[1];
            dest[(i * 6) + 2] = bits[2];
            dest[(i * 6) + 3] = bits[3];
            dest[(i * 6) + 4] = bits[4];
            dest[(i * 6) + 5] = bits[5];
        }
    }

    auto UID::rngDataToBits(unsigned int data) -> std::bitset<32> {
        std::bitset<32> obits;
        for (std::size_t i = 0, off = 31; i < 32; i++, off--) { obits[i] = (data >> off) & 1; }
        return obits;
    }
    auto UID::rngBufferToBits(const std::array<unsigned int, 4> & source, std::bitset<120> & dest) -> void {
        for (std::size_t i = 0; i < source.size(); i++) {
            auto bits = rngDataToBits(source.at(i));
            for (std::size_t j = 0; j < 32; j++) {
                if (((i * 32) + j) >= 120) { break; }
                dest[(i * 32) + j] = bits[j];
            }
        }
    }

} // namespace dracon
