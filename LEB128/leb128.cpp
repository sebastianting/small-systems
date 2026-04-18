#include <cstdint>
#include <iostream>
#include <vector>

std::vector<uint8_t> encode_unsigned(uint64_t num) {
  uint64_t value = num;
  std::vector<uint8_t> result;
  do {
    uint8_t byte = value & 0x7f;
    value >>= 7;
    if (value != 0) {
      byte |= 0x80;
    }
    result.push_back(byte);
  } while (value != 0);

  return result;
}

// we don't need to flip and perform two's compliment ourselves.
// if the caller calls with a negative number it is already two's complimented.
std::vector<uint8_t> encode_signed(int32_t value) {
  bool more = 1;
  std::vector<uint8_t> result;
  while (more) {
    uint8_t byte = value & 0x7f;
    value >>= 7;
    if ((value == 0 && (byte & 0x40) == 0) || // just detect here instead.
        (value == -1 && (byte & 0x40) != 0)) {
      more = false;
    } else {
      byte |= 0x80;
    }
    result.push_back(byte);
  }
  return result;
}

uint64_t decode_unsigned(std::vector<uint8_t>::const_iterator it) {
  uint64_t accumulator = 0;
  int shift = 0;
  for (;;) {
    accumulator |= static_cast<uint64_t>(*it & 0x7f) << shift;
    shift += 7;
    if (!(*it & 0x80)) {
      break;
    }
    it++;
  }
  return accumulator;
}

int64_t decode_signed(std::vector<uint8_t>::const_iterator it) {
  uint64_t accumulator = 0;
  int shift = 0;
  for (;;) {
    accumulator |= static_cast<uint64_t>(*it & 0x7f) << shift;
    shift += 7;
    if (!(*it & 0x80)) {
      break;
    }
    it++;
  }
  if ((shift < 64) && ((*it & 0x40) != 0)) {
    accumulator |= (~0ULL << shift);
  }
  return accumulator;
}

int main() {
  uint64_t a = 232383;
  std::vector<uint8_t> b = encode_unsigned(a);
  std::cout << "Encoded bytes: ";
  for (uint8_t byte : b) {
    std::cout << "0x" << std::hex << static_cast<int>(byte) << " ";
  }

  std::cout << std::endl
            << "Decoded: " << std::dec << decode_unsigned(b.begin())
            << std::endl;

  int64_t c = -232342;
  std::vector<uint8_t> d = encode_signed(c);
  std::cout << std::endl << "Signed encoded: ";
  for (uint8_t byte : d) {
    std::cout << "0x" << std::hex << static_cast<int>(byte) << " ";
  }

  std::cout << std::endl
            << "Decoded: " << std::dec << decode_signed(d.begin()) << std::endl;

  return 0;
}
