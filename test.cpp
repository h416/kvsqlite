#include <cassert>

#include <kvsqlite/kvsqlite.h>
#include <kvsqlite/progress_timer.h>

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

int main(int argc, const char * argv[]) {
    
    kvsqlite::database db("test.sqlite3");
    kvsqlite::progress_timer t;
    
    db.set("hello", "world");

    std::string value;
    db.get("hello", value);

    assert(value == "world");
    
    db.remove("hello");
    bool result = db.exists("hello");
    assert(!result);
    
    Person person("Tim");
    db.set("person1", person);
    assert(db.exists("person1"));
    
    {
        kvsqlite::progress_timer t2;
        kvsqlite::transaction transaction(db);
        for(int i=0;i<10000;++i){
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
        assert(p.name()=="name");
    }
    return 0;
}