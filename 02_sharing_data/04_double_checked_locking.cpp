#include <thread>
#include <mutex>

std::shared_ptr<int> resource_ptr;
std::mutex resource_mutex;

// this IS thread safe because all threads will lock the mutex before querying or changing the resource_ptr, but it's
// also SLOW, because we lock the mutex every time we try to get the resource_ptr
void init_resource() {
  std::lock_guard<std::mutex> lg(resource_mutex);
  if (!resource_ptr) {
    resource_ptr.reset(new int(42));
  }
}

// this IS NOT thread safe because there is a data race, because the read outside the lock (1) is not synchronized with
// the write done by another thread inside the lock (2).
// normally we don't care about this race condition, because if at (1) we don't see the updated value for the
// resource_ptr, we will just acquire the lock and see the updated value in the second check, whilst if we see the
// updated value at (1) we can return.
// the data race might occur if we consider the optimizations the compiler or the hardware might make to our code
// it the case of instruction reordering, the resource_ptr might get assigned a value, before the actual object has
// finished constructing, and thus a call to do_something will be performed on uninitialized data
void undefined_behaviour_with_double_checked_locking() {
  if (!resource_ptr) { // (1)
    std::lock_guard<std::mutex> lg(resource_mutex);
    if (!resource_ptr) {
      resource_ptr.reset(new int(42)); // (2)
    }
  }
  // resource_ptr->do_something(); // (3)
}

std::once_flag resource_flag;

void init_resource_2() {
  resource_ptr.reset(new int(42));
}

void foo() {
  // use of sta::call_once will typically have a lower overhead than using a mutex explicitly, especially when
  // initialization has already been done
  std::call_once(resource_flag, init_resource_2);
  // resource_ptr->do_something();
}

// if this is needed to create a singleton, the best solution is the following
class Singleton {};
Singleton &instance() {
  // initialization is guaranteed to be thread safe and happens only once
  static Singleton instance;
  return instance;
}
