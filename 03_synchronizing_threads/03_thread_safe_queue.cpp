#include <algorithm>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <memory>
#include <queue>
#include <thread>

template <typename T>
class threadsafe_queue
{
private:
  mutable std::mutex _m;
  std::queue<T> _q;
  std::condition_variable _cond;
  bool _production_done{false};

public:
  threadsafe_queue() = default;
  threadsafe_queue(threadsafe_queue const &) = delete;
  threadsafe_queue(threadsafe_queue &&) = delete;
  threadsafe_queue
  operator=(threadsafe_queue const &) = delete;
  threadsafe_queue
  operator=(threadsafe_queue &&) = delete;
  ~threadsafe_queue() = default;

  void
  push(T const &val) {
    std::lock_guard<std::mutex> lk(_m);
    _q.push(val);
    _cond.notify_one();
  }

  bool
  try_pop(T &val) {
    std::lock_guard<std::mutex> lk(_m);

    if (_q.empty()) {
      return false;
    }

    val = _q.front();
    _q.pop();
    return true;
  }

  std::shared_ptr<T>
  try_pop() {
    std::lock_guard<std::mutex> lk(_m);

    if (_q.empty()) {
      return std::shared_ptr<T>(nullptr);
    }

    std::shared_ptr<T> p(std::make_shared(_q.front()));
    _q.pop();
    return p;
  }

  void
  wait_and_pop(T &val) {
    std::unique_lock<std::mutex> lk(_m);
    _cond.wait(lk, [this] { return !_q.empty() || _production_done; });

    if (!_q.empty()) {
      val = _q.front();
      _q.pop();
    }
  }

  std::shared_ptr<T>
  wait_and_pop() {
    std::unique_lock<std::mutex> lk(_m);
    _cond.wait(lk, [this] { return !_q.empty() || _production_done; });

    if (!_q.empty()) {
      std::shared_ptr<T> p(std::make_shared<T>(_q.front()));
      _q.pop();
      return p;
    } else {
      return nullptr;
    }
  }

  bool
  empty() const {
    std::lock_guard<std::mutex> lk(_m);
    return _q.empty();
  }

  size_t
  size() const {
    std::lock_guard<std::mutex> lk(_m);
    return _q.size();
  }

  void
  notify_production_done() {
    std::lock_guard<std::mutex> lk(_m);
    _production_done = true;
    _cond.notify_all();
  }
};

template <typename T>
void
produce_data(threadsafe_queue<T> &q, T begin, T end) {
  for (unsigned d = begin; d < end; ++d) {
    q.push(d);
  }
}

template <typename T>
void
consume_data(threadsafe_queue<T> &q, unsigned id) {
  static std::mutex cout_mtx;

  while (true) {
    auto p = q.wait_and_pop();
    if (p == nullptr) {
      {
        std::lock_guard<std::mutex> lk(cout_mtx);
        std::cout << "Consumer " << id << " stopping\n";
      }
      return;
    } else {
      {
        std::lock_guard<std::mutex> lk(cout_mtx);
        std::cout << "Consumer " << id << " got " << *p << "\n";
      }
    }
  }
}

int
main() {
  static constexpr size_t NUM_PRODUCERS = 20;
  static constexpr size_t NUM_CONSUMERS = 5;
  static constexpr size_t STEP_SIZE = 100000;

  threadsafe_queue<unsigned> q;

  std::vector<std::thread> producers;
  for (unsigned i = 0; i < NUM_PRODUCERS; ++i) {
    producers.emplace_back(
        produce_data<unsigned>, std::ref(q), i * STEP_SIZE, (i + 1) * STEP_SIZE);
  }

  std::vector<std::thread> consumers;
  for (unsigned i = 0; i < NUM_CONSUMERS; ++i) {
    consumers.emplace_back(consume_data<unsigned>, std::ref(q), i);
  }

  std::for_each(
      producers.begin(), producers.end(), std::mem_fn(&std::thread::join));
  q.notify_production_done();
  std::for_each(
      consumers.begin(), consumers.end(), std::mem_fn(&std::thread::join));

  return 0;
}
