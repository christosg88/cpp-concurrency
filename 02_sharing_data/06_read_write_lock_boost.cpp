#include <algorithm>
#include <iostream>
#include <map>
#include <shared_mutex>
#include <string>
#include <thread>
#include <vector>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

class dns_entry {};

class dns_cache {
  std::map<std::string, dns_entry> _entries;
  mutable boost::shared_mutex _entry_mutex;

public:
  dns_entry find_entry(std::string const &domain) const {
    // multiple readers
    boost::shared_lock<boost::shared_mutex> lk(_entry_mutex);
    auto find_it = _entries.find(domain);
    return find_it == _entries.end() ? dns_entry() : find_it->second;
  }

  void update_or_add_entry(std::string const &domain, dns_entry const &dns_details) {
    // single writer
    boost::lock_guard<boost::shared_mutex> lk(_entry_mutex);
    _entries[domain] = dns_details;
  }

  void print_domains() const {
    // multiple readers
    boost::shared_lock<boost::shared_mutex> lk(_entry_mutex);
    for (auto const &p : _entries) {
      std::cout << p.first << "\n";
    }
  }
};

int main() {
  std::array<std::string, 10> const domains{
      "apple.com",
      "youtube.com",
      "www.google.com",
      "play.google.com",
      "microsoft.com",
      "support.google.com",
      "linkedin.com",
      "www.blogger.com",
      "maps.google.com",
      "wordpress.org"};

  dns_cache cache;

  std::vector<std::thread> threads;
  for (std::string const &domain : domains) {
    // create new dns_entry
    dns_entry entry;

    threads.emplace_back(std::thread(&dns_cache::update_or_add_entry, &cache, std::ref(domain), std::ref(entry)));
  }

  std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));

  cache.print_domains();

  return 0;
}
