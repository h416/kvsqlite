cppcheck --enable=all --inconclusive --std=c++11   --force --suppress=missingIncludeSystem  -I include -I cereal-1.1.2/include -I dlib-18.16 -I sqlite-amalgamation-3081002  test.cpp

