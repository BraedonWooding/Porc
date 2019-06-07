#include "table.hpp"

#include <unordered_map>

template<typename Key, typename Value>
using Map = std::unordered_map<Key, Value>;




using BaseTable = Map<
