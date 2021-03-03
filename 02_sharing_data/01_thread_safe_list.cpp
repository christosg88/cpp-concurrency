#include <list>
#include <mutex>
#include <algorithm>

std::list<int> some_list;
std::mutex some_mutex;

void add_to_list(int v) {
  std::lock_guard<std::mutex> lg(some_mutex);
  some_list.push_back(v);
}

bool list_contains(int v) {
  std::lock_guard<std::mutex> lg(some_mutex);
  return std::find(some_list.begin(), some_list.end(), v) != some_list.end();
}
