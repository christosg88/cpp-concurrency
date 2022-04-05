#include <algorithm>
#include <cassert>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string_view>
#include <thread>
#include <vector>

void print_first(std::ostream &os) {
  os << "first ";
}

void print_second(std::ostream &os) {
  os << "second ";
}

void print_third(std::ostream &os) {
  os << "third";
}

class Foo {
private:
  int next;
  std::mutex mtx;
  std::condition_variable cv;

public:
  Foo() : next(1) {}

  void first(std::function<void(std::ostream &)> func, std::ostream &os) {
    std::unique_lock<std::mutex> lck(mtx);
    func(os);
    next = 2;
    lck.unlock();
    cv.notify_all();
  }

  void second(std::function<void(std::ostream &)> func, std::ostream &os) {
    std::unique_lock<std::mutex> lck(mtx);
    cv.wait(lck, [this]() { return this->next == 2; });
    func(os);
    next = 3;
    lck.unlock();
    cv.notify_all();
  }

  void third(std::function<void(std::ostream &)> func, std::ostream &os) {
    std::unique_lock<std::mutex> lck(mtx);
    cv.wait(lck, [this]() { return this->next == 3; });
    func(os);
    next = 0;
  }
};

int main() {
  Foo foo;
  std::ostringstream ss;
  std::vector<std::thread> threads;
  threads.emplace_back(&Foo::third, std::ref(foo), print_third, std::ref(ss));
  threads.emplace_back(&Foo::second, std::ref(foo), print_second, std::ref(ss));
  threads.emplace_back(&Foo::first, std::ref(foo), print_first, std::ref(ss));
  std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));

  std::string_view expected_result{"first second third"};
  assert(expected_result.compare(ss.str()) == 0);
  std::cout << ss.str() << '\n';

  return 0;
}
