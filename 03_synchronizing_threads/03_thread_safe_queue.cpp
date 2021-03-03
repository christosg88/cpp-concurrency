#include <condition_variable>
#include <iostream>
#include <memory>
#include <queue>

template <typename T>
class threadsafe_queue {
private:
  mutable std::mutex _m;
  std::queue<T> _q;
  std::condition_variable _cond;

public:
  threadsafe_queue() {}

  threadsafe_queue(threadsafe_queue const &other) {
    std::lock_guard<std::mutex> lk(other._m);
    _q = other._q;
  }

  threadsafe_queue &operator=(threadsafe_queue const &other) = delete; // disallow copy assignement

  void push(T const &val) {
    std::lock_guard<std::mutex> lk(_m);
    _q.push(val);
    _cond.notify_one();
  }

  bool try_pop(T &val) {
    std::lock_guard<std::mutex> lk(_m);

    if (_q.empty()) {
      return false;
    }

    val = _q.front();
    _q.pop();
    return true;
  }

  std::shared_ptr<T> try_pop() {
    std::lock_guard<std::mutex> lk(_m);

    if (_q.empty()) {
      return std::shared_ptr<T>(nullptr);
    }

    std::shared_ptr<T> p(std::make_shared(_q.front()));
    _q.pop();
    return p;
  }

  void wait_and_pop(T &val) {
    std::unique_lock<std::mutex> lk(_m);
    _cond.wait(lk, [this] { return !_q.empty(); });
    val = _q.front();
    _q.pop();
  }

  std::shared_ptr<T> wait_and_pop() {
    std::unique_lock<std::mutex> lk(_m);
    _cond.wait(lk, [this] { return !_q.empty(); });

    std::shared_ptr<T> p(std::make_shared(_q.front()));
    _q.pop();
    return p;
  }

  bool empty() const {
    std::lock_guard<std::mutex> lk(_m);
    return _q.empty();
  }

  size_t size() const {
    std::lock_guard<std::mutex> lk(_m);
    return _q.size();
  }
};

threadsafe_queue<int> data_queue;

bool more_data_to_prepare() {
  return true;
}

int prepare_data() {
  return 42;
}

void process(int data) {
  std::cout << data << "\n";
}

void data_preparation_thread() {
  while (more_data_to_prepare()) {
    int const data = prepare_data();
    data_queue.push(data);
  }
}

void data_processing_thread() {
  while (true) {
    int data;
    data_queue.wait_and_pop(data);
    process(data);
  }
}
