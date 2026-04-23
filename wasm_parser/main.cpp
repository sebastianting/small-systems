#include <cstdint>
#include <stdexcept>
#include <vector>

struct Reader {
  const std::vector<uint8_t> &size_t pos;
  size_t size;

  uint8_t read_byte() { return data[pos++]; }
  uint32_t read_u32_leb128() {}
}
