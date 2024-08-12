#pragma once
#include <vector>
#include <list>
#include <stack>
#include <queue>
#include <deque>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <string>

#include "Allocator.h"

using namespace std;

template<typename T>
using xvector = vector<T, StlAllocator<T>>;

template<typename T>
using xlist = list<T, StlAllocator<T>>;

template<typename T>
using xdeque = deque<T, StlAllocator<T>>;

template<typename T, typename Container = xdeque<T>>
using xqueue = queue<T, Container>;

template<typename T, typename Container = xdeque<T>>
using xstack = stack<T, Container>;  

template<typename T, typename Container = xvector<T>, typename Pred = less<typename Container::value_type>>
using xpriority_queue = priority_queue < T, Container, Pred>;

template<typename Key, typename T, typename Pred = less<Key>>
using xmap = map<Key, T, Pred, StlAllocator<pair<const Key, T>>>;

template<typename Key, typename Pred = less<Key>>
using xset = set<Key, Pred, StlAllocator<Key>>;

template<typename Key, typename T, typename Hasher = hash<Key>, typename KeyEq = equal_to<Key>>
using xhashmap = unordered_map<Key, T, Hasher, KeyEq, StlAllocator<pair<const Key, T>>>;

template<typename Key, typename Hasher = hash<Key>, typename KeyEq = equal_to<Key>>
using xhashset = unordered_set<Key, Hasher, KeyEq, StlAllocator<Key>>;

using xstring = basic_string<char, char_traits<char>, StlAllocator<char>>;
using xwstring = basic_string<wchar_t, char_traits<wchar_t>, StlAllocator<wchar_t>>;