#include <algorithm> // std::find_if
#include <cstddef> // std::size_t
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <utility>
#include <vector>

template <typename Key, typename Value, typename Hash = std::hash<Key>>
class threadsafe_lut
{
private:
  class bucket_type
  {
  private:
    using bucket_value = std::pair<Key, Value>;
    using bucket_data = std::list<bucket_value>;
    using bucket_iterator = typename bucket_data::iterator;

    bucket_data _data;
    mutable std::shared_mutex _mtx;

    bucket_iterator
    find_entry_for(Key const &key) const {
      return std::find_if(
          _data.begin(), _data.end(), [&](bucket_value const &item) {
            return item.first == key;
          });
    }

  public:
    Value
    value_for(Key const &key, Value const &default_value) const {
      std::shared_lock<std::shared_mutex> lk(_mtx);
      bucket_iterator const found_entry = find_entry_for(key);
      return (found_entry == _data.end()) ? default_value : found_entry->second;
    }

    void
    add_or_update_mapping(Key const &key, Value const value) {
      std::unique_lock<std::shared_mutex> lk(_mtx);
      bucket_iterator const found_entry = find_entry_for(key);
      if (found_entry == _data.end()) {
        _data.push_back(bucket_value(key, value));
      } else {
        found_entry->second = value;
      }
    }

    void
    remove_mapping(Key const &key) {
      std::unique_lock<std::shared_mutex> lk(_mtx);
      bucket_iterator const found_entry = find_entry_for(key);
      if (found_entry != _data.end()) {
        _data.erase(found_entry);
      }
    }
  };

  std::vector<std::unique_ptr<bucket_type>> _buckets;
  Hash _hasher;

  bucket_type &
  get_bucket(Key const &key) const {
    std::size_t const bucket_index = _hasher(key) % _buckets.size();
    return *_buckets[bucket_index];
  }

public:
  using key_type = Key;
  using mapped_type = Value;
  using hash_type = Hash;

  // arbitrary prime number
  static constexpr unsigned NUM_BUCKETS = 19;

  explicit threadsafe_lut(unsigned num_buckets = NUM_BUCKETS,
                          Hash const &hasher = Hash())
      : _buckets(num_buckets),
        _hasher(hasher) {
    for (bucket_type &bucket : _buckets) {
      bucket.reset(new bucket_type);
    }
  }

  ~threadsafe_lut() = default;

  threadsafe_lut(threadsafe_lut const &other) = delete;
  threadsafe_lut &
  operator=(threadsafe_lut const &other) = delete;
  threadsafe_lut(threadsafe_lut &&other) = delete;
  threadsafe_lut &
  operator=(threadsafe_lut &&other) = delete;

  Value
  value_for(Key const &key, Value const &default_value = Value()) const {
    return get_bucket(key).value_for(key, default_value);
  }

  void
  add_or_update_mapping(Key const &key, Value const &value) {
    get_bucket(key).add_or_update_mapping(key, value);
  }

  void
  remove_mapping(Key const &key) {
    get_bucket(key).remove_mapping(key);
  }

  std::map<Key, Value>
  get_map() const {
    std::vector<std::unique_lock<std::shared_mutex>> locks;
    locks.reserve(_buckets.size());
    std::transform(_buckets.begin(),
                   _buckets.end(),
                   std::back_inserter(locks),
                   [](auto const &bucket) { return bucket._mtx; });

    std::map<Key, Value> res;
    for (bucket_type const &bucket : _buckets) {
      for (typename bucket_type::bucket_value const &kv : bucket._data) {
        res.insert(kv);
      }
    }

    return res;
  }
};
