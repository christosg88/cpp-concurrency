#include <future>
#include <iostream>
#include <string>

int find_the_answer_to_ltuae();

void do_other_stuff();

int main() {
  std::future<int> the_answer = std::async(find_the_answer_to_ltuae);
  do_other_stuff();
  std::cout << "The answer is " << the_answer.get() << std::endl;
}

// run in a new thread
auto answer1 = std::async(std::launch::async, find_the_answer_to_ltuae);
// run when wait() or get() method is called
auto answer2 = std::async(std::launch::deferred, find_the_answer_to_ltuae);

struct X {
  void foo(int, std::string const &);
  std::string bar(std::string const &);
};

X x;
// calls p->foo(42, "hello") where p is &x
auto f1 = std::async(&X::foo, &x, 42, "hello");
// calls tmpx.bar("goodbye") where tmpx is a copy of x
auto f2 = std::async(&X::bar, x, "goodbye");

struct Y {
  double operator()(double);
};

Y y;
// calls tmpy(3.1415) where tmpy is move constructed from Y()
auto f3 = std::async(Y(), 3.1415);
// calls y(3.1415)
auto f4 = std::async(std::ref(y), 3.1415);

X baz(X &) {
  return X();
}
// calls bax(x)
auto f5 = std::async(baz, std::ref(x));

class move_only {
public:
  move_only();
  move_only(move_only &&) = default;
  move_only(move_only const &) = delete;
  move_only &operator=(move_only &&) = default;
  move_only &operator=(move_only const &) = delete;
  void operator()();
};
// calls tmp() where tmp is constructed from std::move(move_only())
auto f6 = std::async(move_only());
