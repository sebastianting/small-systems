#pragma once
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>

class reader {

private:
  const std::span<const uint8_t> data;

  size_t pos = 0;

public:
  bool at_end() const { return pos >= data.size(); }

  reader(const std::vector<uint8_t> &v) : data(v) {}
  reader(std::span<const uint8_t> s) : data(s) {}

  reader sub_reader(size_t length) {
    if (pos + length > data.size())
      throw std::runtime_error("subreader is too long");

    reader sub(data.subspan(pos, length));
    pos += length;
    return sub;
  }

  uint8_t read_byte() {
    if (at_end())
      throw std::runtime_error("unexpected end of input");

    return data[pos++];
  }

  void skip(size_t n) { pos += n; }

  uint64_t read_u64_leb128() {
    uint64_t accumulator = 0;
    int shift = 0;
    for (;;) {
      uint8_t byte = read_byte();
      accumulator |= static_cast<uint64_t>(byte & 0x7f) << shift;
      shift += 7;
      if (!(byte & 0x80)) {
        break;
      }
    }
    return accumulator;
  }

  uint32_t read_u32_leb128() {
    return static_cast<uint32_t>(read_u64_leb128());
  }

  int64_t read_s64_leb128() {
    uint64_t accumulator = 0;
    int shift = 0;
    uint8_t byte = 0;
    for (;;) {
      byte = read_byte();
      accumulator |= static_cast<uint64_t>(byte & 0x7f) << shift;
      shift += 7;
      if (!(byte & 0x80)) {
        break;
      }
    }
    if ((shift < 64) && ((byte & 0x40) != 0)) {
      accumulator |= (~0ULL << shift);
    }
    return static_cast<int64_t>(accumulator);
  }

  int32_t read_s32_leb128() { return static_cast<int32_t>(read_s64_leb128()); }
};

class parser {
private:
  reader reader;
  std::string output;

public:
  parser(const std::vector<uint8_t> &data) : reader(data) {}

  void parse() {
    parse_header();
    while (!reader.at_end()) {
      parse_section();
    }
  }

  void parse_header() {}

  void parse_section() {
    uint8_t id = reader.read_byte();
    uint32_t size = reader.read_u32_leb128();

    switch (id) {
    case 1:
      parse_type_section(size);
      break;
    default:
      reader.skip(size);
    }
    // include section_end passing
  }

  void parse_type_section(size_t size) {
    class reader r = reader.sub_reader(size);

    uint32_t count = r.read_byte();
    for (uint32_t i = 0; i < count; i++) {
      if (r.read_byte() != 0x60) {
        throw std::runtime_error("type section invalid form");
      }

      uint32_t param_count = r.read_u32_leb128();
      for (int j = 0; j < param_count; j++) {
        uint8_t type = r.read_byte();
      }

      uint32_t result_count = r.read_u32_leb128();
      for (int j = 0; j < result_count; j++) {
        uint8_t type = r.read_byte();
      }
    }

    // TODO: turn these types into (i32) like outputs,
    // probably match them into a dictionary with the opcodes and such
  }
};
