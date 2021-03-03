#include <deque>
#include <future>
#include <mutex>
#include <thread>
#include <utility>

std::mutex m;
// the type of tasks would be double(int, float&) if it expected functions with return type double and argumentts an int
// and a float reference
std::deque<std::packaged_task<void()>> tasks;

bool gui_shutdown_message_received();

void get_and_process_gui_message();

void gui_thread() {
  while (!gui_shutdown_message_received()) {
    get_and_process_gui_message();
    std::packaged_task<void()> task;

    {
      std::lock_guard<std::mutex> lk(m);
      if (tasks.empty()) {
        continue;
      }
      task = std::move(tasks.front());
      tasks.pop_front();
    }

    task();
  }
}

std::thread gui_bg_thread(gui_thread);

template <typename Func>
std::future<void> post_task_for_gui_thread(Func f) {
  std::packaged_task<void()> task(f);
  std::future<void> res = task.get_future();
  std::lock_guard<std::mutex> lk(m);
  tasks.push_back(std::move(task));
  return res;
}
