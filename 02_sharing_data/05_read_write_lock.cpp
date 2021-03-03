#include <map>
#include <mutex>
#include <shared_mutex>
#include <string>

class dns_entry {};

class dns_cache {
  std::map<std::string, dns_entry> _entries;
  mutable std::shared_mutex _entry_mutex;

public:
  dns_entry find_entry(std::string const &domain) const {
    std::shared_lock<std::shared_mutex> lk(_entry_mutex);
    auto find_it = _entries.find(domain);
    return find_it == _entries.end() ? dns_entry() : find_it->second;
  }

  void update_or_add_entry(std::string const &domain, dns_entry const &dns_details) {
    std::lock_guard<std::shared_mutex> lk(_entry_mutex);
    _entries[domain] = dns_details;
  }
};
