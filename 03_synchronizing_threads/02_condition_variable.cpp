#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

std::mutex mut;
std::queue<int> data_queue;
std::condition_variable data_cond;

bool more_data_to_prepare() {
  return true;
}

int prepare_data() {
  return 42;
}

void process(int data) {
  std::cout << data << "\n";
}

bool is_last_chunk(int data) {
  return false;
}

void data_preparation_thread() {
  while (more_data_to_prepare()) {
    int const data = prepare_data();
    {
      std::lock_guard<std::mutex> lk(mut);
      data_queue.push(data);
    }
    data_cond.notify_one();
  }
}

void data_processing_thread() {
  while (true) {
    std::unique_lock<std::mutex> lk(mut);
    data_cond.wait(lk, [] { return !data_queue.empty(); });
    int data = data_queue.front();
    data_queue.pop();
    lk.unlock();
    process(data);

    if (is_last_chunk(data)) {
      break;
    }
  }
}
