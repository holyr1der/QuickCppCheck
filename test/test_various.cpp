#include <iostream>
#include <limits>
#include <algorithm>

#include "../quickcppcheck/property.hpp"

template<typename T>
std::ostream& operator<<(std::ostream& out, const std::vector<T> & v)
{
    std::cout<<"V<";
    for (size_t i = 0;i < v.size();++i) {
        out<<v[i]<<", ";
    }
    out<<">";
    return out;
}

template<typename T, typename U>
std::ostream& operator<<(std::ostream &out, const std::map<T,U> &v)
{
    return std::cout<<"Too complex, screw it...";
}

struct MyArbitrary {
    int operator()(){
        if (rand() % 100 == 1)
            return 666;
        return (rand() % 100) * 10;
    }
};

struct fun {
    bool operator()(int &x) {
        return x % 10 == 0;
    }
};

using namespace QuickCppCheck;

void test_various()
{
    Property<int>([] (int x) { return 2 * x  / 2 == x; })();

    Property<int>([] (int x) { return x / 2  * 2 == x; },
        "This one should fail on odd numbers!!")
    ();

    (Property<std::string&>([] (std::string &s) -> bool
            { std::string tmp = s;
              std::reverse(s.begin(), s.end());
              std::reverse(s.begin(), s.end());
              //intentional bug!
              //this Property should fail when input starts with 'ab'!
              //if (s[0] == 'a' ) s[0] = 'b';
              return tmp == s;
            },
        "reverse cancels reverse"))
    (10000);

    (Property<std::vector<int>>([] (std::vector<int> v)
            { return v.size() <= 2 || (v.size() > 0 && (v[0] <= 2000000000 ));},
            "Testing vector of ints, this should randomly fail.", 0)
        | [](const std::vector<int> &v) { return v.size() < 31; })
    (30);

    (Property<std::vector<std::string>&>([] (std::vector<std::string> &v)
            { return v.size() <= 2 || (v.size() > 0 && (v[0]  != "a"));},
        "Testing vector of strings, this should randomly fail.", 0)
        .Rnd<0>(1,6)
        | [](const std::vector<std::string> &v) { return v.size() < 31; })
    (30);

    (Property<int&>(fun(),
        "This fails with input 666.")
        .Rnd<0>(MyArbitrary()))
    (1000);

    (Property<int, std::string>([] (int x, std::string s)
                                {return true;},
        "Test", 2)
        .One<0>({1,2,3,4,5,-8})
        .Frq<1>({{"Hello", 2}, {"Bye", 8}}))
    (10);

    //Solving Euler problem 9!
    //will, hopefully, fail with the solution!
    (Property<int, int, int, int>([] (int a, int b, int c, int d)
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
        .Fix<3>(1000))
        //run and hopefully will hit the solution
    (100000000);

    (Property<std::map<std::vector<int>,std::string>>([](const std::map<std::vector<int>,std::string>&m){
            return true;},
        "Check that we accept complex types")
        .Fix<0>({{{0,1},"test"}, {{1,2,3}, "haha"}})
        .Frq<0>({{{{{0,1},"test"}, {{1,2,3}, "haha"}},2}}))
    (100);
}

int main()
{
    std::cout<<"======================================"<<std::endl;
    std::cout<<"Running various tests..."<<std::endl;
    test_various();
    std::cout<<"===========tests end=================="<<std::endl;
}

