#include <memory>
#include <mutex>

template <typename T>
class threadsafe_list {
  struct node {
    std::mutex _m;
    std::shared_ptr<T> _data;
    std::unique_ptr<node> _next;

    node() = default;
    explicit node(T value) : _data(std::make_shared<T>(std::move(value))) {}
    explicit node(T &&value) : _data(std::make_shared<T>(value)) {}
  };

  node _head;

public:
  threadsafe_list() = default;
  ~threadsafe_list() {
    remove_if([](node const &/*unused*/) { return true; });
  }

  threadsafe_list(threadsafe_list const &other) = delete;
  threadsafe_list(threadsafe_list &&other) = delete;
  threadsafe_list &operator=(threadsafe_list const &other) = delete;
  threadsafe_list &operator=(threadsafe_list &&other) = delete;

  void push_front(T value) {
    std::unique_ptr<node> new_node(new node(std::move(value)));
    std::lock_guard<std::mutex> lk(_head._m);
    new_node->_next = std::move(_head._next);
    _head._next = std::move(new_node);
  }

  void push_front(T &&value) {
    std::unique_ptr<node> new_node(new node(std::move(value)));
    std::lock_guard<std::mutex> lk(_head._m);
    new_node->_next = std::move(_head._next);
    _head._next = std::move(new_node);
  }

  template <typename Function>
  void for_each(Function f) {
    node *current = &_head;
    std::unique_lock<std::mutex> lk(_head._m);
    while (node *const next = current->_next.get()) {
      std::unique_lock<std::mutex> next_lk(next->_m);
      lk.unlock();
      f(*next->_data);
      current = next;
      lk = std::move(next_lk);
    }
  }

  template <typename Predicate>
  std::shared_ptr<T> find_first_if(Predicate p) {
    node *current = &_head;
    std::unique_lock<std::mutex> lk(_head._m);
    while (node *const next = current->_next.get()) {
      std::unique_lock<std::mutex> next_lk(next->_m);
      lk.unlock();
      if (p(*next->_data)) {
        return next->_data;
      }
      current = next;
      lk = std::move(next_lk);
    }
    return nullptr;
  }

  template <typename Predicate>
  void remove_if(Predicate p) {
    node *current = &_head;
    std::unique_lock<std::mutex> lk(_head._m);
    while (node *const next = current->_next.get()) {
      std::unique_lock<std::mutex> next_lk(next->_m);
      if (p(*next->_data)) {
        current->_next = std::move(next->_next);
        next_lk.unlock();
      } else {
        lk.unlock();
        current = next;
        lk = std::move(next_lk);
      }
    }
  }
};
