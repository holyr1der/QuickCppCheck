#include "../quickcppcheck/property.hpp"

int f(int i, double d)
{
    return i > 5;
}

template<typename T>
std::ostream& operator<<(std::ostream& out, std::vector<T> & v)
{
    std::cout<<"V<";
    for (size_t i = 0;i < v.size();++i) {
        out<<v[i]<<" ";
    }
    out<<">";
    return out;
}


bool sum(int a, int b, int c, int d) {
    return a + b + c != d;
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

int main()
{
    srand(time(NULL));

    //should pass
    Property<int&>([] (int &x) { return 2 * x  / 2 == x; })();

    (Property<char, char&>([] (char x, char &y) { return 2 * ((x + 1) / 2) == x; })
        < Acceptor<char, 0>([] (char &x) { return x < 90 && x > 80;})
        < Acceptor<char, 1>([] (char &x) { return x > 40 && x < 50;})) ();

    (Property<std::string>([] (std::string s) -> bool 
            { std::string tmp = s;
              std::reverse(s.begin(), s.end());
              std::reverse(s.begin(), s.end());
              //intentional bug!
              //this Property should fail when input starts with 'ab'!
              if (s[0] == 'a' && s[1] == 'b') s[0] = 'b';
              return tmp == s;
            },
            "reverse cancels reverse")
        < Acceptor<std::string, 0>([] (std::string &s)
            { for (size_t i = 0;i < s.size();++i)
                if (s[i] < 32 || s[i] > 126)
                    return false;
              return true;
            }))
        (100000);

    Property<int, double&, std::string>(
            [] (int x, double &d, std::string s) { return 2 * x / 2 == x; })
        (100000);

    (Property<std::vector<int>>([] (std::vector<int> v)
            { return v.size() <= 2 || (v.size() > 0 && (v[0] <= 2000000000 ));},
            "vector of ints",
            0)
        < Acceptor<std::vector<int>, 0>([] (std::vector<int> &v)
            { return v.size() < 31; }))
        (300000);

    (Property<int, int, int, int>(sum)
        <  Acceptor<int, 0>([] (int &x) { return x >= 666;})
        <  Acceptor<int, 1>([] (int &x) { return x >= 666;})
        <  Acceptor<int, 2>([] (int &x) { return x >= 666;})
        <= Arbitrary<int, 0>(MyArbitrary())
        <= Arbitrary<int, 1>(MyArbitrary())
        )
        (10000);

    (Property<int&>(fun())
        <= Arbitrary<int, 0>(MyArbitrary()))
        (1000);

    //Solving Eurler problem 9!
    //will, hopefully, fail with the solution!
    (Property<int, int, int, int>([](int a, int b, int c, int d){
                                          return (a*a + b*b != c*c) ||
                                                  (a + b + c != d);
                                            },
                                        "Euler problem 9!!")
        //a, b and c can't be bigger than 500
        <= Arbitrary<int, 0>([]() {return rand() % 500;})
        <= Arbitrary<int, 1>([]() {return rand() % 500;})
        <= Arbitrary<int, 2>([]() {return rand() % 500;})
        //d must be always 1000
        <= Arbitrary<int, 3>([]() {return 1000;})
        //run 1000000 times and hopefully will hit the solution
    )
        (10000000);

    return 0;
}

