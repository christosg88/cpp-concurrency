#include <algorithm>
#include <functional>
#include <iostream>
#include <thread>
#include <vector>

void say_hello() {
  std::cout << "Hello, world!\n";
}

int main() {
  std::vector<std::thread> threads;
  for (int i = 0; i < 20; ++i) {
    threads.push_back(std::thread(say_hello));
  }
  std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));

  return 0;
}
