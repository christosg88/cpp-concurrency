#include <iostream>
#include <thread>

class background_task {
public:
  background_task() {
    std::cout << "Constructor called\n";
  }

  background_task(background_task const &other) {
    std::cout << "Copy constructor called\n";
  }

  background_task(background_task &&other) {
    std::cout << "Move constructor called\n";
  }

  void operator()() const {
    std::cout << "Function call operator called\n";
  }
};

int main() {
  background_task f;
  // this will create a copy of f into the storage belonging to the newly created thread!
  std::thread t1(f);
  t1.join();

  // this will move the background_task object to the storage belonging to the newly created thread
  std::thread t2{background_task()};
  t2.join();

  // this will move too
  std::thread t3(std::move(f));
  t3.join();

  return 0;
}
