#include <exception>
#include <memory>
#include <mutex>
#include <stack>

struct empty_stack : std::exception {
  char const *what() const throw();
};

template <typename T>
class threadsafe_stack {
private:
  std::stack<T> _data;
  mutable std::mutex _m;

public:
  /// Default constructor
  threadsafe_stack() {}

  /// Copy constructor
  threadsafe_stack(threadsafe_stack const &other) {
    std::lock_guard<std::mutex> lg(other._m);
    _data = other._data;
  }

  /// Copy assignment operator
  threadsafe_stack &operator=(threadsafe_stack const &) = delete;

  void push(T new_value) {
    std::lock_guard<std::mutex> lg(_m);
    _data.push(std::move(new_value));
  }

  std::shared_ptr<T> pop() {
    std::lock_guard<std::mutex> lg(_m);
    if (_data.empty()) {
      throw empty_stack();
    }

    std::shared_ptr<T> const res(std::make_shared<T>(std::move(_data.top())));
    _data.pop();
    return res;
  }

  void pop(T &value) {
    std::lock_guard<std::mutex> lg(_m);
    if (_data.empty()) {
      throw empty_stack();
    }
    value = std::move(_data.top());
    _data.pop();
  }

  bool empty() const {
    std::lock_guard<std::mutex> lg(_m);
    return _data.empty();
  }
};
