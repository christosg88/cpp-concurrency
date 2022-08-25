#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <atomic>
#include <algorithm>
#include <functional>

std::atomic<size_t> remaining_producers = 4;
std::mutex mut;
std::queue<int> data_queue;
std::condition_variable data_cond;

void data_preparation_thread() {
  for (int i = 0; i < 10; ++i) {
    {
      std::lock_guard<std::mutex> lk(mut);
      data_queue.push(i);
    }
    data_cond.notify_one();
  }
  --remaining_producers;
}

void data_processing_thread() {
  while (true) {
    int data = 0;
    {
      std::unique_lock<std::mutex> lk(mut);
      if (remaining_producers == 0 && data_queue.empty()) {
        return;
      }
      data_cond.wait(lk, [] { return !data_queue.empty(); });
      data = data_queue.front();
      data_queue.pop();
      std::cout << "Consumer " << std::this_thread::get_id() << " got " << data << '\n';
      lk.unlock();
    }
  }
}

int main() {
  std::cout << "One producer, four consumers\n";
  remaining_producers = 1;
  std::vector<std::thread> threads;
  threads.emplace_back(data_preparation_thread);
  for (unsigned p = 0; p < 4; ++p) {
    threads.emplace_back(data_processing_thread);
  }
  std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));
  threads.clear();

  std::cout << "Four producers, one consumer\n";
  remaining_producers = 4;
  for (unsigned p = 0; p < 4; ++p) {
    threads.emplace_back(data_preparation_thread);
  }
  threads.emplace_back(data_processing_thread);
  std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));
  threads.clear();

  std::cout << "Four producers, four consumers\n";
  remaining_producers = 4;
  for (unsigned p = 0; p < 4; ++p) {
    threads.emplace_back(data_preparation_thread);
  }
  for (unsigned p = 0; p < 4; ++p) {
    threads.emplace_back(data_processing_thread);
  }
  std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));

  return 0;
}
