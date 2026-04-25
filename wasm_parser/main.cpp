#include "parser.hpp"

#include <vector>
#include <fstream>

std::vector<uint8_t> load_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    return std::vector<uint8_t>(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );
}

int main() {
  auto bytes = load_file("fib.wasm");
  parser p(bytes);
  p.parse();
}
