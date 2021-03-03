#include <mutex>
#include <vector>

template <typename T>
class BigVector {
public:
  std::vector<T> _v;

  BigVector() : _v(10'000) {}

  BigVector(BigVector const &other) : _v(other.v) {}
};

template <typename T>
class X {
private:
  BigVector<T> _bv;
  std::mutex _m;

public:
  X(BigVector<T> const &bv) : _bv(bv) {}

  friend void swap(X &lhs, X &rhs) {
    if (&lhs == &rhs) {
      return;
    }

    // lock the two mutexes at the same time
    std::lock(lhs._m, rhs._m);
    // create two lock guards, passing adopt lock so they know to now try to lock the mutex again, but to adopt the
    // ownership of the existing lock
    std::lock_guard<std::mutex> lg1(lhs._m, std::adopt_lock);
    std::lock_guard<std::mutex> lg2(rhs._m, std::adopt_lock);
    std::swap(lhs._bv, rhs._bv);
  }
};

// the same lock pattern but using C++17 scoped_lock
template <typename T>
class X17 {
private:
  BigVector<T> _bv;
  std::mutex _m;

public:
  X17(BigVector<T> const &bv) : _bv(bv) {}

  friend void swap(X17 &lhs, X17 &rhs) {
    if (&lhs == &rhs) {
      return;
    }

    // when the constructor of scoped_lock completes, all mutexes passed as parameters are locked
    std::scoped_lock sl(lhs._m, rhs._m);
    std::swap(lhs._bv, rhs._bv);
    // the destructor of scoped_lock unlocks all locked mutexes
  }
};

