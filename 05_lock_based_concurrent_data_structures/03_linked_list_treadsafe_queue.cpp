#include <algorithm>
#include <cassert>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

template <typename T>
class threadsafe_queue {
private:
  struct node {
    std::shared_ptr<T> _data;
    std::unique_ptr<node> _next;
  };

  std::unique_ptr<node> _head;
  std::mutex _head_mtx;
  node *_tail;
  std::mutex _tail_mtx;
  std::condition_variable _data_cond;

  node *get_tail() {
    std::lock_guard<std::mutex> tail_lk(_tail_mtx);
    return _tail;
  }

  std::unique_ptr<node> pop_head() {
    std::unique_ptr<node> old_head = std::move(_head);
    _head = std::move(old_head->_next);
    return old_head;
  }

  std::unique_lock<std::mutex> wait_for_data() {
    std::unique_lock<std::mutex> head_lk(_head_mtx);
    _data_cond.wait(head_lk, [&] { return _head.get() != get_tail(); });
    return head_lk;
  }

  std::unique_ptr<node> wait_pop_head() {
    std::unique_lock<std::mutex> head_lk(wait_for_data());
    return pop_head();
  }

  std::unique_ptr<node> wait_pop_head(T &value) {
    std::unique_lock<std::mutex> head_lk(wait_for_data());
    value = std::move(*_head->_data);
    return pop_head();
  }

public:
  // create a dummy node on creation to ensure there's always at least one node
  // in the queue to separate the node being accessed at the head from that
  // being accessed at the tail
  threadsafe_queue() : _head(new node), _tail(_head.get()) {}
  ~threadsafe_queue() = default;

  // no copy constructor
  threadsafe_queue(threadsafe_queue const &other) = delete;
  // no move constructor
  threadsafe_queue(threadsafe_queue &&other) = delete;
  // no copy assignment operator
  threadsafe_queue &operator=(threadsafe_queue const &other) = delete;
  // no move assignment operator
  threadsafe_queue &operator=(threadsafe_queue &&oher) = delete;

  /// Pop from the head of the queue
  std::shared_ptr<T> try_pop() {
    std::unique_ptr<node> old_head = pop_head();
    return old_head != nullptr ? old_head->_data : std::shared_ptr<T>();
  }

  std::shared_ptr<T> wait_and_pop() {
    std::unique_ptr<node> const old_head = wait_pop_head();
    return old_head->_data;
  }

  void wait_and_pop(T &value) {
    std::unique_ptr<node> const old_head = wait_pop_head(value);
  }

  /// Push to the tail of the queue
  void push(T new_value) {
    std::shared_ptr<T> new_data(std::make_shared<T>(std::move(new_value)));
    std::unique_ptr<node> p(new node);
    {
      std::lock_guard<std::mutex> tail_lk(_tail_mtx);
      _tail->_data = new_data;
      node *const new_tail = p.get();
      _tail->_next = std::move(p);
      _tail = new_tail;
    }
    _data_cond.notify_one();
  }
};

void enqueue_jobs(threadsafe_queue<int> &queue, int from, int size) {
  for (int i = from, to = from + size; i < to; ++i) {
    queue.push(i);
  }
}

int main() {
  threadsafe_queue<int> q;

  std::vector<std::thread> threads;
  auto num_threads = std::thread::hardware_concurrency();

  constexpr int batch_size = 1000000;
  for (unsigned int t = 0; t < num_threads - 1; ++t) {
    threads.emplace_back(enqueue_jobs, std::ref(q), t * batch_size, batch_size);
  }

  unsigned int count = 0;
  int val = 0;
  while (true) {
    q.wait_and_pop(val);
    std::cout << count++ << ": " << val << "\n";
  }

  std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));

  assert(count == num_threads * batch_size);

  return 0;
}
