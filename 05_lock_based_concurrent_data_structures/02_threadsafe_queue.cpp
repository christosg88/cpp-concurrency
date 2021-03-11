#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

template <typename T>
class threadsafe_queue {
private:
  mutable std::mutex _m;
  std::queue<std::shared_ptr<T>> _data;
  std::condition_variable _cond;

public:
  threadsafe_queue() {}

  void push(T value) {
    std::shared_ptr<T> data(std::make_shared<T>(std::move(value)));
    std::lock_guard<std::mutex> lg(_m);
    _data.push(data);
    _cond.notify_one();
  }

  void wait_and_pop(T &value) {
    std::unique_lock<std::mutex> lk(_m);
    _cond.wait(lk, [this] { return !_data.empty(); });
    value = std::move(*_data.front());
    _data.pop();
  }

  std::shared_ptr<T> wait_and_pop() {
    std::unique_lock<std::mutex> lk(_m);
    _cond.wait(lk, [this] { return !_data.empty(); });
    std::shared_ptr<T> res(_data.front());
    _data.pop();
    return res;
  }

  bool try_pop(T &value) {
    std::lock_guard<std::mutex> lk(_m);
    if (_data.empty()) {
      return false;
    }
    value = std::move(*_data.front());
    _data.pop();
    return true;
  }

  std::shared_ptr<T> try_pop() {
    std::lock_guard<std::mutex> lk(_m);
    if (_data.empty()) {
      return std::shared_ptr<T>();
    }
    std::shared_ptr<T> res(_data.front());
    _data.pop();
    return res;
  }

  bool empty() const {
    std::lock_guard<std::mutex> lk(_m);
    return _data.empty();
  }
};
