#include <iostream>
#include <thread>

class Object {
public:
  void say_hello() {
    std::cout << "Hello, world!\n";
  }
};

int main() {
  Object ob;
  std::thread t(&Object::say_hello, &ob);
  t.join();

  return 0;
}
