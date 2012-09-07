#include <iostream>
#include <limits>
#include <algorithm>

#include "../../quickcppcheck/property.hpp"

namespace std {

template<typename T, typename U>
std::ostream& operator<<(std::ostream &out, const std::map<T,U> &v)
{
    return std::cout<<"Too complex, screw it...";
}

}

struct MyArbitrary {
    int operator()(){
        if (rand() % 100 == 1)
            return 666;
        return (rand() % 100) * 10;
    }
};

bool prop_test(int a) {
    return true;
}

int my_rand() {
    return rand() % 100;
}

struct fun {
    bool operator()(int &x) {
        return x % 10 == 0;
    }
};

using namespace qcppc;

void test_various()
{
    property([] (int x) { return 2 * x  / 2 == x; })();

    property([] (int x) { return x / 2  * 2 == x; },
        "This one should fail on odd numbers!!")
    ();

    property([] (std::string &s) -> bool
            { std::string tmp = s;
              std::reverse(s.begin(), s.end());
              std::reverse(s.begin(), s.end());
              //intentional bug!
              //this property should fail when input starts with 'ab'!
              //if (s[0] == 'a' ) s[0] = 'b';
              return tmp == s;
            },
        "reverse cancels reverse")
    (10000);

    property([] (std::vector<int> v)
            { return v.size() <= 2 || (v.size() > 0 && (v[0] <= 2000000000 ));},
            "Testing vector of ints, this should randomly fail.", 0)
        .If([](const std::vector<int> &v) { return v.size() < 31; })
    (30);

    property([] (std::vector<std::string> &v)
            { return v.size() <= 2 || (v.size() > 0 && (v[0]  != "a"));},
        "Testing vector of strings, this should randomly fail.", 0)
        .Rnd<0>(1,6)
        .If([](const std::vector<std::string> &v) { return v.size() < 31; })
    (30);

    property(fun(),
        "This fails with input 666.")
        .Rnd<0>(MyArbitrary())
    (1000);

    property([] (int x, std::string s)
                                {return true;},
        "Test", 2)
        .One<0>({1,2,3,4,5,-8})
        .Frq<1>({{"Hello", 2}, {"Bye", 8}})
        .Classify([](int x, const std::string & s)
                { return s; })
    (10);

    //Solving Euler problem 9!
    //will, hopefully, fail with the solution!
    property([] (int a, int b, int c, int d)
                    {
                        return (a*a + b*b != c*c) ||
                            (a + b + c != d);
                    },
        "Euler problem 9!!")
        //a, b and c can't be bigger than 500
        .Rnd<0>(1, 500)
        .Rnd<1>(1, 500)
        .Rnd<2>(1, 500)
        //d must be always 1000
        .Fix<3>(1000)
        //run and hopefully will hit the solution
    (100000000);

    property([](const std::map<std::vector<int>,std::string>&m){
            return true;},
        "Check that we accept complex types")
        .Fix<0>({{{0,1},"test"}, {{1,2,3}, "haha"}})
        .Frq<0>({{{{{0,1},"test"}, {{1,2,3}, "haha"}},2}})
    (100);

    property([] (std::vector<char> & v1, std::vector<char> &v2) {
                            if (std::accumulate(v1.begin(), v1.end(),0,
                                    [](char x, char y) {return x^y;}) ==
                                std::accumulate(v2.begin(), v2.end(),0,
                                    [](char x, char y) {return x^y;})) {
                                sort(v1.begin(), v1.end());
                                sort(v2.begin(), v2.end());
                                return v1 == v2;
                            }
                            return true;
                            },
        "xor check")
        .Rnd<0>(Arbitrary<char>('a','z'), 26, 26)
        .Fix<1>([](){
            std::vector<char> v;
            for (char i = 'a';i <= 'z';++i) v.push_back(i);
            return v;
            }())
    (1000);

    property([](const std::string &s) { return true;},
            "Test strings", 1)
        .Classify([](const std::string &s) {
                if (s.size() < 10) return "shorter than 10";
                else if  (s.size() < 20) return "shorter than 20";
                else return "big ones"; })
        (20);

    property(&prop_test)
        .Rnd<0>(my_rand)();

    property(prop_test)
        .Rnd<0>(my_rand)();

    property(std::function<bool(bool,int)>([](bool,int){return true;}))
        .Rnd<1>(my_rand)
        .Classify([](bool b, int) { return b?"true":"false";})
    ();
}

#ifndef NOMAIN
int main()
#else
int main0()
#endif
{
    std::cout<<"======================================"<<std::endl;
    std::cout<<"Running various tests..."<<std::endl;
    test_various();
    std::cout<<"===========tests end=================="<<std::endl;
}
