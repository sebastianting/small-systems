#pragma once
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <span>
#include <stdexcept>
#include <vector>

class Reader {

private:
  const std::span<const uint8_t>
      data; // we use spans so that we can split the viewing window easier.

  size_t pos = 0;

public:
  bool at_end() const { return pos >= data.size(); }

  Reader(const std::vector<uint8_t> &v) : data(v) {}
  Reader(std::span<const uint8_t> s) : data(s) {}

  Reader sub_reader(size_t length) {
    if (pos + length > data.size())
      throw std::runtime_error("subreader window past data end");

    Reader sub(data.subspan(pos, length));
    pos += length;
    return sub;
  }

  uint8_t read_byte() {
    if (at_end())
      throw std::runtime_error("unexpected end of input");

    return data[pos++];
  }

  void skip(size_t n) {
    if (pos + n > data.size())
      throw std::runtime_error("skip past end");
    pos += n;
  }

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

struct func_type {
  std::vector<uint8_t> params;
  std::vector<uint8_t> results;
};

class parser {
private:
  Reader reader;
  std::string output;

  std::vector<func_type> types;
  std::vector<uint32_t> defined_functions;

public:
  parser(const std::vector<uint8_t> &data) : reader(data) {}

  void parse() {
    parse_header();
    while (!reader.at_end()) {
      parse_section();
    }
  }

  void parse_header() {
    uint32_t magic = reader.read_byte() | reader.read_byte() << 8 |
                     reader.read_byte() << 16 | reader.read_byte() << 24;
    uint32_t version = reader.read_byte() | reader.read_byte() << 8 |
                       reader.read_byte() << 16 | reader.read_byte() << 24;
    if (magic != 0x6D736100)
      throw std::runtime_error("Invalid magic header");
    if (version != 0x00000001)
      throw std::runtime_error("Invalid Version");
  }

  void parse_section() {
    uint8_t id = reader.read_byte();
    uint32_t size = reader.read_u32_leb128();

    switch (id) {
    case 1: {
      parse_type_section(size);
      print_types(types);
      break;
    }
    case 3:
      parse_function_section(size);
      break;
    default:
      reader.skip(size);
    }
    // include section_end passing
  }

  void parse_type_section(size_t size) {
    Reader r = reader.sub_reader(size);

    uint32_t count = r.read_u32_leb128();

    for (uint32_t i = 0; i < count; i++) {
      if (r.read_byte() != 0x60) {
        throw std::runtime_error("type section invalid form");
      }

      func_type t;

      uint32_t param_count = r.read_u32_leb128();
      for (uint32_t j = 0; j < param_count; j++) {
        t.params.push_back(r.read_byte());
      }

      uint32_t result_count = r.read_u32_leb128();
      for (uint32_t j = 0; j < result_count; j++) {
        t.results.push_back(r.read_byte());
      }
      types.push_back(t);
    }
  }

  void parse_function_section(size_t size) {
    Reader r = reader.sub_reader(size);

    uint32_t count = r.read_u32_leb128();

    for (uint32_t i = 0; i < count; i++) {
      defined_functions.push_back(r.read_u32_leb128());
    }
  }

  // TODO: other sections

  // -----------------------------output-----------------------------------------

  std::string type_to_string(uint8_t t) {
    switch (t) {
    case 0x7F:
      return "i32";
    case 0x7E:
      return "i64";
    case 0x7D:
      return "f32";
    case 0x7C:
      return "f64";
    default:
      return "unknown";
    }
  }

  void print_types(const std::vector<func_type> &types) {
    for (size_t i = 0; i < types.size(); i++) {
      std::cout << "(type " << i << " (func";

      if (!types[i].params.empty()) {
        std::cout << " (param";
        for (auto p : types[i].params)
          std::cout << " " << type_to_string(p);

        std::cout << ")";
      }

      if (!types[i].results.empty()) {
        std::cout << " (result";
        for (auto p : types[i].results)
          std::cout << " " << type_to_string(p);

        std::cout << ")";
      }
      std::cout << "))\n";
    }
  }
};
