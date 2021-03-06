# kvsqlite

[![Build Status](https://travis-ci.org/h416/kvsqlite.svg?branch=master)](https://travis-ci.org/h416/kvsqlite)

kvsqlite is a header-only C++ library designed to make simple key value store using SQLite.
It was inspired by [YapDatabase](https://github.com/yapstudios/YapDatabase).
It was build on [SQLite](https://sqlite.org), [dlib](https://github.com/davisking/dlib) and [cereal](https://github.com/USCiLab/cereal).


# Tutorial

## Quick Start
```cpp
#include <kvsqlite/kvsqlite.h>

kvsqlite::database db("test.sqlite3");

db.set("hello", "world");

std::string value;
db.get("hello", value);

assert(value == "world");
```

## Defining data model
kvsqlite uses [cereal](https://github.com/USCiLab/cereal) to serialize the objects to JSON.
```cpp
class Person
{
public:
    Person(const std::string& name):_name(name){}
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

Person person("Tim");

db.set("person1", person);

assert(db.exists("person1")==true);
```


## Transaction
```cpp
{
    kvsqlite::transaction transaction(db);
    for(int i=0;i<10000;++i){
        Person p("name");
        std::string key = std::to_string(i);
        db.set(key, p);
    }
    transaction.commit();
}
```
# License
kvsqlite is licensed under [Boost Software License](http://www.boost.org/LICENSE_1_0.txt).


