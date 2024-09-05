#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

void get_data(std::atomic<bool> &data_ready, std::vector<int> &data) {
  // spinlock
  while (!data_ready.load());
  std::cout << data[0] << '\n';
}

void set_data(std::atomic<bool> &data_ready, std::vector<int> &data) {
  using namespace std::chrono_literals;
  std::this_thread::sleep_for(200ms);
  data.push_back(42);
  data_ready.store(true);
}

int main() {
  std::atomic<bool> data_ready{false};
  std::vector<int> data;

  std::jthread t1(get_data, std::ref(data_ready), std::ref(data));
  std::jthread t2(set_data, std::ref(data_ready), std::ref(data));

  return 0;
}
