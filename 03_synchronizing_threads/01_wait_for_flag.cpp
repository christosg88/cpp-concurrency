#include <mutex>
#include <thread>

bool flag;
std::mutex m;

// DON'T DO THAT!
// this is better that continuously checking the flag without going to sleep, which would waste CPU time, but it's hard
// to get the sleep duration right and the thread still wastes CPU time checking
void wait_for_flag() {
  std::unique_lock<std::mutex> lk(m);
  while (!flag) {
    lk.unlock();
    // sleep for 100ms
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    lk.lock();
  }
}

int main() {
  wait_for_flag();
  return 0;
}
