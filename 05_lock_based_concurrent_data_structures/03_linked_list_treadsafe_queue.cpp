#include <iostream>
#include <memory>

template <typename T>
class queue {
private:
  struct node {
    T _data;
    std::unique_ptr<node> _next;
    explicit node(T data) : _data(std::move(data)) {}
  };

  std::unique_ptr<node> _head;
  node *_tail;

public:
  queue() : _tail(nullptr) {}
  ~queue() = default;

  // no copy constructor
  queue(queue const &other) = delete;
  // no move constructor
  queue(queue &&other) = delete;
  // no copy assignment operator
  queue &operator=(queue const &other) = delete;
  // no move assignment operator
  queue &operator=(queue &&oher) = delete;

  /// Pop from the head of the queue
  std::shared_ptr<T> try_pop() {
    // if the queue is empty return a nullptr
    if (_head == nullptr) {
      return std::shared_ptr<T>();
    }

    std::shared_ptr<T> res(std::make_shared<T>(std::move(_head->_data)));
    // move the current _head node to a temporary unique_ptr so it's deleted when it goes out of scope
    std::unique_ptr<node> const old_head = std::move(_head);
    _head = std::move(old_head->_next);

    // if this was the last node in the queue, _tail was also pointing to it, so we must reset _tail too
    if (_head == nullptr) {
      _tail = nullptr;
    }
    return res;
  }

  /// Push to the tail of the queue
  void push(T new_value) {
    // create a new node inside a unique_ptr to store the new value
    std::unique_ptr<node> p(new node(std::move(new_value)));
    node *const new_tail = p.get();

    if (_tail != nullptr) {
      // _next always points towards the tail of the queue
      // so when we add a new node in the queue, make sure _tail's _next points to the new node
      _tail->_next = std::move(p);
    } else {
      _head = std::move(p);
    }

    _tail = new_tail;
  }
};

int main() {
  queue<int> q;

  for (int i = 0; i < 5; ++i) {
    q.push(i);
  }

  std::shared_ptr<int> p;
  while ((p = q.try_pop()) != nullptr) {
    std::cout << *p << std::endl;
  }

  return 0;
}
