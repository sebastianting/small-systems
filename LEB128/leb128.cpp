#include <cstdint>
#include <iostream>
#include <vector>

std::vector<uint8_t> encode(uint64_t num) {
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

uint64_t decode(std::vector<uint8_t>::const_iterator it) {
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

int main() {
  uint64_t a = 232383;
  std::vector<uint8_t> b = encode(a);
  std::cout << "Encoded bytes: ";
  for (uint8_t byte : b) {
    std::cout << "0x" << std::hex << static_cast<int>(byte) << " ";
  }

  std::cout << std::endl
            << "Decoded: " << std::dec << decode(b.begin()) << std::endl;

  return 0;
}
