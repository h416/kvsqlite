
//      Copyright Shinichiro Hirama 2015.
// Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//      http://www.boost.org/LICENSE_1_0.txt)

#ifndef INCLUDE_KVSQLITE_KVSQLITE_H_
#define INCLUDE_KVSQLITE_KVSQLITE_H_

#include <sstream>
#include <string>
#include <memory>

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
      update_statement_->bind(1, data);
      update_statement_->bind(2, metadata);
      update_statement_->bind(3, rowid);
      update_statement_->exec();
    }else{
      insert_statement_->bind(1, collection);
      insert_statement_->bind(2, key);
      insert_statement_->bind(3, data);
      insert_statement_->bind(4, metadata);
      insert_statement_->exec();
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
    get_statement_->bind(1, collection);
    get_statement_->bind(2, key);
    get_statement_->exec();

    get_statement_->move_next();
    if(1 < get_statement_->get_num_columns()){
      get_statement_->get_column(0, data);
      if(metadata){
        get_statement_->get_column(1, *metadata);
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
  void remove(const std::string& key, const std::string& collection = "")
  {
    remove_statement_->bind(1, collection);
    remove_statement_->bind(2, key);
    remove_statement_->exec();
  }
  void removeAllObjectsInCollection(const std::string& collection)
  {
    dlib::statement st(*this, "DELETE FROM \"kvsqlite\" WHERE \"collection\" = ?;");
    st.bind(1, collection);
    st.exec();
  }
  void removeAll()
  {
    exec("DELETE FROM \"kvsqlite\";");
  }

  int64 numberOfCollections()
  {
    dlib::statement st(*this, "SELECT COUNT(DISTINCT collection) AS NumberOfRows FROM \"kvsqlite\";");
    return count(st);
  }
  int64 numberOfKeysInCollection(const std::string& collection)
  {
    dlib::statement st(*this, "SELECT COUNT(*) AS NumberOfRows FROM \"kvsqlite\" WHERE \"collection\" = ?;");
    st.bind(1, collection);
    return count(st);
  }
  int64 numberOfKeysInAllCollections()
  {
    dlib::statement st(*this, "SELECT COUNT(*) AS NumberOfRows FROM \"kvsqlite\";");
    return count(st);
  }

 private:
  std::unique_ptr<dlib::statement> find_statement_;
  std::unique_ptr<dlib::statement> get_statement_;
  std::unique_ptr<dlib::statement> update_statement_;
  std::unique_ptr<dlib::statement> insert_statement_;
  std::unique_ptr<dlib::statement> remove_statement_;
    
  void init()
  {
      create_table();
      init_statements();
  }
  void init_statements()
  {
      find_statement_.reset (new dlib::statement(*this, "SELECT \"rowid\" FROM \"kvsqlite\" WHERE \"collection\" = ? AND \"key\" = ?;") );
      
      get_statement_.reset (new dlib::statement(*this, "SELECT \"data\", \"metadata\" FROM \"kvsqlite\" WHERE \"collection\" = ? AND \"key\" = ?;"));
      
      update_statement_.reset (new dlib::statement(*this, "UPDATE \"kvsqlite\" SET \"data\" = ?, \"metadata\" = ? WHERE \"rowid\" = ?;"));
      
      insert_statement_.reset (new dlib::statement(*this, "INSERT INTO \"kvsqlite\""
                         " (\"collection\", \"key\", \"data\", \"metadata\") VALUES (?, ?, ?, ?);"));
      
      remove_statement_.reset (new dlib::statement(*this, "DELETE FROM \"kvsqlite\" WHERE \"collection\" = ? AND \"key\" = ?;"));
  }
  bool table_exists(const std::string& tablename)
  {
    return dlib::query_int(*this, "select count(*) from sqlite_master where name = \""+tablename+"\"") == 1;
  }

  dlib::int64 find(const std::string& key, const std::string& collection = "")
  {
    find_statement_->bind(1, collection);
    find_statement_->bind(2, key);
    find_statement_->exec();

    find_statement_->move_next();
    dlib::int64 rowid = 0;
    if(0 < find_statement_->get_num_columns())
    {
      rowid = find_statement_->get_column_as_int64(0);
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

  int64 count(dlib::statement &st)
  {
    st.exec();
    st.move_next();
    dlib::int64 result = 0;
    if(0 < st.get_num_columns())
    {
      result = st.get_column_as_int64(0);
    }
    return result;
  }
};

}  // namespace kvsqlite

#endif  // INCLUDE_KVSQLITE_KVSQLITE_H_
