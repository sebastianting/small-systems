#include "parser.hpp"

#include <vector>
/*
std::vector<uint8_t> load_file(const std::string& path)  {

}*/

int main() {
  std::vector<uint8_t> bytes = {0}; // load_file("fib.wasm");
  parser p(bytes);
  p.parse();
}
