#include <atomic>

class spinlock_mutex {
  std::atomic_flag flag;

public:
  spinlock_mutex(): flag(ATOMIC_FLAG_INIT) {}

  void lock() {
    // keep spinning on the flag, until the old value is false, indicating that
    // this thread set the value to true
    // test_and_set() atomically changes the state of a std::atomic_flag to set
    // (true) and returns the value it held before
    while(flag.test_and_set(std::memory_order_acquire));
  }

  void unlock() {
    // clear the flag back to the unset (false) state
    flag.clear(std::memory_order_release);
  }
};
