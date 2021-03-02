#include <iostream>
#include <thread>

class Object {
public:
  Object() {
    std::cout << "Object() called\n";
  }

  Object(Object const &other) {
    std::cout << "Object(Object const &other) called\n";
  }
};

void func1(Object ob) {
  std::cout << "func1(Object ob) called\n";
}

void func2(Object &ob) {
  std::cout << "func2(Object &ob) called\n";
}

void func3(Object &&ob) {
  std::cout << "func3(Object &&ob) called\n";
}

int main() {
  // calling default constructor
  Object ob1;
  // calling copy constructor twice
  // 1. when ob1 is copied to the memory storage of t1
  // 2. when the copy is copied again as an argument of func1
  std::thread t1(func1, ob1);
  t1.join();

  // calling default constructor
  Object ob2;
  // no copy is made
  std::thread t2(func2, std::ref(ob2));
  t2.join();

  // calling default constructor
  Object ob3;
  // calling copy constructor once
  // 1. when ob3 is copied to the memory storage of t3
  // then an rvalue reference is forwarded to func3
  std::thread t3(func3, ob3);
  t3.join();

  // calling default constructor
  Object ob4;
  // no copy is made
  std::thread t4([&ob4]() { func2(ob4); });
  t4.join();

  return 0;
}
