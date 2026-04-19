#include <iostream>
#include <string>
#include <unordered_map>

struct node {
  std::string value;
  std::string key;
  struct node *prev;
  struct node *next;
};

class lru_cache {
private:
  int capacity;
  std::unordered_map<std::string, node *> cache;
  node *head; // these are dummmy nodes
  node *tail;

  void remove(node *n) {
    n->prev->next = n->next;
    n->next->prev = n->prev;
    n->prev = nullptr;
    n->next = nullptr;
    return;
  }

  void insert_front(node *n) {
    n->next = head->next;
    n->prev = head;
    head->next->prev = n;
    head->next = n;
  }

public:
  lru_cache(int capacity) {
    this->capacity = capacity;
    head = new node{"void", "head", nullptr, nullptr};
    tail = new node{"void", "tail", nullptr, nullptr};
    head->next = tail;
    tail->prev = head;
  }

  std::string get(std::string key) {
    auto it = cache.find(key);
    if (it == cache.end()) {
      return "not found!";
    }
    node *n = it->second;
    remove(n);
    insert_front(n);

    return n->value;
  }

  void put(std::string key, std::string value) {
    auto it = cache.find(key);

    if (it != cache.end()) {
      node *n = it->second;
      n->value = value;
      remove(n);
      insert_front(n);
    } else {
      if (capacity == 0) {
        return;
      }
      if (cache.size() == capacity) {
        node *lru = tail->prev;
        cache.erase(lru->key);
        remove(lru);
        delete lru;
      }
      node *n = new node{value, key, nullptr, nullptr};
      insert_front(n);
      cache[key] = n;
    }
  }

  void print() {
    node *curr = head->next;
    while (curr != tail) {
      std::cout << curr->key << ": " << curr->value << " -> ";
      curr = curr->next;
    }
    std::cout << std::endl;
  }

  ~lru_cache() {
    node *curr = head;
    while (curr != nullptr) {
      node *next = curr->next;
      delete (curr);
      curr = next;
    }
    cache.clear();
  }
};

int main() {
  lru_cache a(5);
  a.put("a", "1");
  a.print();
  a.put("b", "2");
  a.put("c", "3");
  a.put("d", "4");
  a.put("e", "5");
  a.print();
  a.put("f", "6");
  a.print();
  a.put("c", "coriander");
  a.print();
  a.put("z", "zebra");
  a.print();
  return 0;
}
