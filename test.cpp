#include <kvsqlite/kvsqlite.h>
#include <cassert>

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

    db.set("hello", "world");

    std::string value;
    db.get("hello", value);

    assert(value == "world");
    
    Person person("Tim");
    db.set("person1", person);
    assert(db.exists("person1"));
    
    return 0;
}