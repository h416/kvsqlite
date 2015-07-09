#include <cassert>
#include <string>
#include <vector>

#include <kvsqlite/kvsqlite.h>
#include <kvsqlite/progress_timer.h>

class Person
{
 public:
  explicit Person(const std::string& name):_name(name){}
  std::string name() const {return _name;}
 private:
  std::string _name;

  friend class cereal::access;

  template<class Archive>
  void serialize(Archive & ar)
  {
    ar(cereal::make_nvp("name", _name));
  }
};

int main(int argc, const char * argv[])
{
  kvsqlite::database db("test.sqlite3");
  kvsqlite::progress_timer t;

  db.set("hello", "world");

  std::string value;
  db.get("hello", value);

  assert(value == "world");

  db.remove("hello");
  bool result = db.exists("hello");
  assert(!result);

  db.set("hello", "kvsqlite");
  result = db.get("hello", value);
  assert(result);
  assert(value == "kvsqlite");

  db.del("hello");
  result = db.has_key("hello");
  assert(!result);

  Person person("Tim");
  db.set("person1", person);
  assert(db.exists("person1"));

  {
    kvsqlite::progress_timer t2;
    kvsqlite::transaction transaction(db);
    for(int i = 0; i < 10000; ++i){
      Person p("name");
      std::string key = std::to_string(i);
      db.set(key, p);
    }
    transaction.commit();
  }

  {
    kvsqlite::progress_timer t2;
    Person p("");
    result = db.get("1", p);
    assert(result);
    assert(p.name() == "name");
  }

  db.clear();
  assert(db.size() == 0);

  db.set("key", "value", "collection");
  db.set("key", "value", "collection2");
  db.set("key2", "value2", "collection2");
  assert(db.size() == 3);
  assert(db.collection_count() == 2);
  assert(db.count("collection") == 1);
  assert(db.count("collection2") == 2);
  std::vector<std::string> collections = db.collections();
  assert(collections.size() == 2);
  assert(collections[0] == "collection");
  assert(collections[1] == "collection2");

  std::vector<std::string> keys = db.keys("collection2");
  assert(keys.size() == 2);
  assert(keys[0] == "key");
  assert(keys[1] == "key2");

  db.remove_collection("collection2");
  assert(db.collection_count() == 1);
  assert(db.size() == 1);
  return 0;
}
