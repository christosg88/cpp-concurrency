#include <iostream>
#include <thread>

class thread_guard {
private:
  std::thread &_t;

public:
  explicit thread_guard(std::thread &t) : _t(t) {}

  ~thread_guard() {
    if (_t.joinable()) {
      _t.join();
    }
  }

  thread_guard(thread_guard const &) = delete; // no copy constructor
  thread_guard operator=(thread_guard const &) = delete; // no copy assignment operator
};

struct func {
  int &_i;

  func(int &i) : _i(i) {}

  void operator()() {
    for (unsigned j = 0; j < 1'000'000; ++j) {
      std::cout << _i << "\n";
    }
  }
};

int main() {
  int some_local_var = 50;
  func my_func(some_local_var);
  std::thread t(my_func);

  // this would bring some_local_var out of scope and the _i reference inside
  // my_func would be dangling
  // t.detach();

  // this will ensure that t is joined when g is destructed, so no dangling
  // reference is possible
  thread_guard g(t);

  return 0;
}
