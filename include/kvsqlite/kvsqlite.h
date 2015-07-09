
//      Copyright Shinichiro Hirama 2015.
// Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//      http://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDE_KVSQLITE_KVSQLITE_H_
#define INCLUDE_KVSQLITE_KVSQLITE_H_

#include <sstream>
#include <string>
#include <memory>
#include <vector>

#include <dlib/sqlite.h>

#include <cereal/archives/json.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/types/bitset.hpp>
// #include <cereal/types/boost_variant.hpp>
#include <cereal/types/chrono.hpp>
#include <cereal/types/common.hpp>
#include <cereal/types/complex.hpp>
#include <cereal/types/deque.hpp>
#include <cereal/types/forward_list.hpp>
#include <cereal/types/list.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/queue.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/stack.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/tuple.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/unordered_set.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/valarray.hpp>
#include <cereal/types/vector.hpp>

namespace kvsqlite {

// typedefs
typedef dlib::int64 int64;
typedef dlib::transaction transaction;

// json
template<typename T>
inline std::string to_json(const T& obj)
{
  std::ostringstream os;
  {
    cereal::JSONOutputArchive archive(os);
    archive(CEREAL_NVP(obj));
  }
  return os.str();
}
inline std::string to_json(const std::string& s)
{
  return s;
}
inline std::string to_json(const char* s)
{
  return std::string(s);
}

template<typename T>
inline void from_json(const std::string &str, T& obj)
{
  std::istringstream is(str);
  cereal::JSONInputArchive archive(is);
  archive(obj);
}


// database
class database : public dlib::database
{
 public:
  database(){}
  explicit database(const std::string& file):dlib::database(file)
  {
    init();
  }
  void open(const std::string& file)
  {
    dlib::database::open(file);
    init();
  }
  bool has_key(const std::string& key, const std::string& collection = "")
  {
    return exists(key, collection);
  }
  bool exists(const std::string& key, const std::string& collection = "")
  {
    return find(key, collection) > 0;
  }
  template<class T>
  void set(const std::string& key, const T& data, const std::string& collection = "", const std::string& metadata = "")
  {
    std::string json = to_json(data);
    set(key, json, collection, metadata);
  }
  void set(const std::string& key, const std::string& data, const std::string& collection = "", const std::string& metadata = "")
  {
    dlib::int64 rowid = find(key, collection);

    if(rowid > 0){
      dlib::statement* st = update_statement();
      st->bind(1, data);
      st->bind(2, metadata);
      st->bind(3, rowid);
      st->exec();
    }else{
      dlib::statement* st = insert_statement();
      st->bind(1, collection);
      st->bind(2, key);
      st->bind(3, data);
      st->bind(4, metadata);
      st->exec();
    }
  }
  template<class T>
  bool get(const std::string& key, T& data, const std::string& collection = "", std::string* metadata = nullptr)
  {
    std::string json;
    bool result = get(key, json, collection, metadata);
    if(result){
      from_json(json, data);
    }
    return result;
  }
  bool get(const std::string& key, std::string& data, const std::string& collection = "", std::string* metadata = nullptr)
  {
    dlib::statement* st = data_statement();
    st->bind(1, collection);
    st->bind(2, key);
    st->exec();

    st->move_next();
    if(1 < st->get_num_columns()){
      st->get_column(0, data);
      if(metadata){
        st->get_column(1, *metadata);
      }
      return true;
    }else{
      data.clear();
      if(metadata){
        metadata->clear();
      }
      return false;
    }
  }
  // Deletes the database row with the given key/collection.
  void del(const std::string& key, const std::string& collection = "")
  {
    remove(key, collection);
  }
  void remove(const std::string& key, const std::string& collection = "")
  {
    dlib::statement* st = remove_statement();
    st->bind(1, collection);
    st->bind(2, key);
    st->exec();
  }
  // Deletes every key/object pair from the given collection.
  void remove_collection(const std::string& collection)
  {
    dlib::statement* st = remove_collection_statement();
    st->bind(1, collection);
    st->exec();
  }
  // Removes every key/object pair in the entire database (from all collections).
  void clear()
  {
    dlib::statement* st = clear_statement();
    st->exec();
  }

  // Returns the total number of collections.
  int64 collection_count()
  {
    dlib::statement* st = collection_count_statement();
    return count(st);
  }
  // Returns the total number of keys in the given collection.
  int64 count(const std::string& collection)
  {
    dlib::statement* st = count_statement();
    st->bind(1, collection);
    return count(st);
  }
  // Returns the total number of key/object pairs in the entire database (including all collections).
  int64 size()
  {
    dlib::statement* st = size_statement();
    return count(st);
  }

  // Returns a list of all collection names.
  std::vector<std::string> collections()
  {
    return get_vector(collections_statement());
  }

  // Returns a list of all keys in the given collection.
  std::vector<std::string> keys(const std::string &collection = "")
  {
    dlib::statement* st = keys_statement();
    st->bind(1, collection);
    return get_vector(st);
  }

 private:
  std::unique_ptr<dlib::statement> find_statement_;
  std::unique_ptr<dlib::statement> data_statement_;
  std::unique_ptr<dlib::statement> update_statement_;
  std::unique_ptr<dlib::statement> insert_statement_;
  std::unique_ptr<dlib::statement> remove_statement_;
  std::unique_ptr<dlib::statement> remove_collection_statement_;
  std::unique_ptr<dlib::statement> clear_statement_;
  std::unique_ptr<dlib::statement> collection_count_statement_;
  std::unique_ptr<dlib::statement> count_statement_;
  std::unique_ptr<dlib::statement> size_statement_;
  std::unique_ptr<dlib::statement> collections_statement_;
  std::unique_ptr<dlib::statement> keys_statement_;

  void init()
  {
      create_table();
  }
  dlib::statement* statement(std::unique_ptr<dlib::statement> &st, const std::string &sql)
  {
    if(!st){
      st.reset(new dlib::statement(*this, sql));
    }
    return st.get();
  }
  dlib::statement* find_statement()
  {
    return statement(find_statement_, "SELECT \"rowid\" FROM \"kvsqlite\" WHERE \"collection\" = ? AND \"key\" = ?;");
  }
  dlib::statement* data_statement()
  {
    return statement(data_statement_, "SELECT \"data\", \"metadata\" FROM \"kvsqlite\" WHERE \"collection\" = ? AND \"key\" = ?;");
  }
  dlib::statement* update_statement()
  {
    return statement(update_statement_, "UPDATE \"kvsqlite\" SET \"data\" = ?, \"metadata\" = ? WHERE \"rowid\" = ?;");
  }
  dlib::statement* insert_statement()
  {
    return statement(update_statement_, "INSERT INTO \"kvsqlite\""
                         " (\"collection\", \"key\", \"data\", \"metadata\") VALUES (?, ?, ?, ?);");
  }
  dlib::statement* remove_statement()
  {
    return statement(remove_statement_, "DELETE FROM \"kvsqlite\" WHERE \"collection\" = ? AND \"key\" = ?;");
  }
  dlib::statement* remove_collection_statement()
  {
    return statement(remove_collection_statement_, "DELETE FROM \"kvsqlite\" WHERE \"collection\" = ?;");
  }
  dlib::statement* clear_statement()
  {
    return statement(clear_statement_, "DELETE FROM \"kvsqlite\";");
  }
  dlib::statement* collection_count_statement()
  {
    return statement(collection_count_statement_, "SELECT COUNT(DISTINCT collection) AS NumberOfRows FROM \"kvsqlite\";");
  }
  dlib::statement* count_statement()
  {
    return statement(count_statement_, "SELECT COUNT(*) AS NumberOfRows FROM \"kvsqlite\" WHERE \"collection\" = ?;");
  }
  dlib::statement* size_statement()
  {
    return statement(size_statement_, "SELECT COUNT(*) AS NumberOfRows FROM \"kvsqlite\";");
  }
  dlib::statement* collections_statement()
  {
    return statement(collections_statement_, "SELECT DISTINCT \"collection\" FROM \"kvsqlite\";");
  }
  dlib::statement* keys_statement()
  {
    return statement(keys_statement_, "SELECT \"key\" FROM \"kvsqlite\" WHERE \"collection\" = ?;");
  }

  bool table_exists(const std::string& tablename)
  {
    return dlib::query_int(*this, "SELECT COUNT(*) FROM SQLITE_MASTER WHERE NAME = \""+tablename+"\"") == 1;
  }

  dlib::int64 find(const std::string& key, const std::string& collection = "")
  {
    dlib::statement* st = find_statement();
    st->bind(1, collection);
    st->bind(2, key);
    st->exec();

    st->move_next();
    dlib::int64 rowid = 0;
    if(0 < st->get_num_columns())
    {
      rowid = st->get_column_as_int64(0);
    }
    return rowid;
  }

  void create_table()
  {
    if (!table_exists("kvsqlite")){
      exec("CREATE TABLE IF NOT EXISTS \"kvsqlite\""
         " (\"rowid\" INTEGER PRIMARY KEY,"
         "  \"collection\" CHAR NOT NULL,"
         "  \"key\" CHAR NOT NULL,"
         "  \"data\" CHAR NOT NULL,"
         "  \"metadata\" CHAR NOT NULL"
         " );");
      exec("CREATE UNIQUE INDEX IF NOT EXISTS \"true_primary_key\" ON \"kvsqlite\" ( \"collection\", \"key\" );");
    }
  }

  int64 count(dlib::statement *st)
  {
    st->exec();
    st->move_next();
    dlib::int64 result = 0;
    if(0 < st->get_num_columns())
    {
      result = st->get_column_as_int64(0);
    }
    return result;
  }
  std::vector<std::string> get_vector(dlib::statement *st)
  {
    std::vector<std::string> result;
    st->exec();
    while(st->move_next())
    {
      std::string collection;
      st->get_column(0, collection);
      result.push_back(collection);
    }
    return result;
  }
};

}  // namespace kvsqlite

#endif  // INCLUDE_KVSQLITE_KVSQLITE_H_
