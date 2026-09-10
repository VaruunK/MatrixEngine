#pragma once

#include <unordered_map>
#include <utility>

template<class Key, class T>
class HashMap {
public:

    using key_type = typename std::unordered_map<Key, T>::key_type;
    using mapped_type = typename std::unordered_map<Key, T>::mapped_type;
    using value_type = typename std::unordered_map<Key, T>::value_type;
    using size_type = typename std::unordered_map<Key, T>::size_type;
    using iterator = typename std::unordered_map<Key, T>::iterator;
    using const_iterator = typename std::unordered_map<Key, T>::const_iterator;

    HashMap() = default;

    // element access
    T& operator[](const Key& key) { return map[key]; }
    T& operator[](Key&& key) { return map[std::move(key)]; }

    T& at(const Key& key) { return map.at(key); }
    const T& at(const Key& key) const { return map.at(key); }

    // iterators
    iterator begin() { return map.begin(); }
    const_iterator begin() const { return map.begin(); }
    iterator end() { return map.end(); }
    const_iterator end() const { return map.end(); }

    // capacity
    bool empty() const { return map.empty(); }
    size_type size() const { return map.size(); }

    // modifiers
    void clear() { map.clear(); }

    std::pair<iterator, bool> insert(const value_type& value) {
        return map.insert(value);
    }

    template<class... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        return map.emplace(std::forward<Args>(args)...);
    }

    size_type erase(const Key& key) { return map.erase(key); }
    iterator erase(iterator pos) { return map.erase(pos); }

    // lookup
    iterator find(const Key& key) { return map.find(key); }
    const_iterator find(const Key& key) const { return map.find(key); }

    size_type count(const Key& key) const { return map.count(key); }
    bool contains(const Key& key) const { return map.find(key) != map.end(); }

private:
    std::unordered_map<Key, T> map;
};