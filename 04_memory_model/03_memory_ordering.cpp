#include <assert.h>
#include <atomic>
#include <iostream>
#include <thread>

// sequentially consistent ordering
//   memory_order_seq_cst (default and the most stringent)
// acquire-release ordering
//   memory_order_consume
//   memory_order_acquire
//   memory_order_release
//   memory_order_acq_rel
// relaxed ordering
//   memory_order_relaxed

// The default ordering is named sequentially consistent because it implies that
// the behavior of the program is consistent with a simple sequential view of
// the world. If all operations on instances of atomic types are sequentially
// consistent, the behavior of a multithreaded program is as if all these
// operations were performed in some particular sequence by a single thread.

std::atomic<bool> x, y;
std::atomic<int> z;

void write_x() {
  x.store(true, std::memory_order_seq_cst); // (1)
}

void write_y() {
  y.store(true, std::memory_order_seq_cst); // (2)
}

void read_x_then_y() {
  while (!x.load(std::memory_order_acq_rel))
    ;
  if (y.load(std::memory_order_seq_cst)) { // (3)
    ++z;
  }
}

void read_y_then_x() {
  while (!y.load(std::memory_order_acq_rel))
    ;
  if (x.load(std::memory_order_seq_cst)) { // (4)
    ++z;
  }
}

void write_x_then_y_relaxed() {
  x.store(true, std::memory_order_relaxed); // (5)
  y.store(true, std::memory_order_relaxed); // (6)
}

void read_y_then_x_relaxed() {
  while (!y.load(std::memory_order_relaxed))
    ; // (7)
  if (x.load(std::memory_order_relaxed)) { // (8)
    ++z;
  }
}

int main() {
  x = false;
  y = false;
  z = 0;

  std::thread a(write_x);
  std::thread b(write_y);
  std::thread c(read_x_then_y);
  std::thread d(read_y_then_x);
  a.join();
  b.join();
  c.join();
  d.join();

  // since the order is sequentially consistent z is asserted to be either 1 or
  // 2 at the end of the program.
  // 1. If none of x or y is set, we can finish main because threads c and d are
  //    waiting in the while loops. So at least one of x and y must be set.
  // 2. If x is set and y is not set, (3) will not update z. But since in order
  //    to reach (3) x had to be set, (4) will increment z when y gets set.
  // 3. If x is not set and y is set, (4) will not update z. But since in order
  //    to reach (4) y had to be set, (3) will increment z when x gets set.
  // 4. If both x and y are set, both (3) and (4) will atomically increment z,
  //    resulting in z == 2 at the end.
  assert(z.load() != 0);

  x = false;
  y = false;
  z = 0;

  std::thread e(write_x_then_y_relaxed);
  std::thread f(read_y_then_x_relaxed);
  e.join();
  f.join();

  // this time assert can fire, because the load of x (8) can read false, even
  // though the read of y (7) reads true and the store of x (5) happens before
  // the store of y (6).
  assert(z.load() != 0);
  std::cout << z.load() << std::endl;

  return 0;
}
