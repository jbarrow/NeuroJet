#if !defined(BINDLIST_HPP)
#   define BINDLIST_HPP

#include <string>
#include <map>
using std::string;
using std::map;
using std::pair;

template<class T> class BindList {
public:
   BindList() {};
   ~BindList() {};
   void insert(const string &k, const T &AddObj, const bool &IsReadOnly = false) {
      dataMap[k] = pair<T, bool>(AddObj, IsReadOnly);
   }
   inline void remove(const string &k) { dataMap.erase(k); }
   inline bool exists(const string &k) const {
      return (dataMap.find(k) != dataMap.end());
   }
   inline const T & GetEntry(const string &k) const {
      return dataMap.find(k)->second.first;
   }
   inline T & GetEntry(const string &k) {
      return dataMap.find(k)->second.first;
   }
   inline bool IsReadOnly(const string &k) const {
      return dataMap.find(k)->second.second;
   }

private:
   map<string, pair<T, bool> > dataMap;
};

#endif
