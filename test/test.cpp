#include <iostream>
#include <set>
#include <algorithm>

#include "../quickcppcheck/property.hpp"

#define _1K   1000
#define _10K  10000
#define _100K 100000
#define _1M   1000000
#define _10M  10000000
#define _100M 100000000
#define _1G   1000000000

using namespace qcppc;

template<typename T>
struct prop_mean
{
    int NTESTS;
    double EXPECTED, TOL;
    bool DEBUG;
    double sum;
    int count ;
    prop_mean(int NTESTS, double EXPECTED, double TOL, bool DEBUG = false):
        NTESTS(NTESTS), EXPECTED(EXPECTED), TOL(TOL), DEBUG(DEBUG),sum(0),count(0) {}
    bool operator()(T v) {
        count++;
        sum += v;
        if (count == NTESTS) {
            if (DEBUG)
                std::cout<<"Sum: "<<sum<<"  Mean: "<<sum/count<<std::endl;
            return fabs(sum / count - EXPECTED) < TOL;
        }
        return true;
    }
};


void test_arbit()
{
    property(prop_mean<int>( _10M, 0, 1000000, true),
            "Mean value of Arbitrary<int> should be close to 0")
        .Rnd<0>(std::numeric_limits<int>::min(),
                    std::numeric_limits<int>::max())
    (_10M);

    /* 
    (property(prop_mean<long>( _10M, 0, 10000, true),
            "Mean value of Arbitrary<long> should be close to 0")
        )
    (_10M);
    */
    property(prop_mean<short>( _10M, 0, 100, true),
            "Mean value of Arbitrary<short> should be close to 0")
    (_10M);

    property(prop_mean<char>( _10M, 0, 1, true),
            "Mean value of Arbitrary<char> should be close to 0")
    (_10M);

}

void test_arbit_bounded()
{
    property([](int a) { return a <= 100 && a >= -100;},
        "Arbitrary<int>(-100, 100) should return between -100 and 100 inclusive")
        .Rnd<0>(-100, 100)
    (_10M);

    property([](unsigned int a) { return a <= 1000 && a >= 0;},
        "Arbitrary<unsigned int>(0, 1000) should return between 0 and 1000 inclusive")
        .Rnd<0>(0, 1000)
    (_10M);

    property([] (short n)
                     { return n <= std::numeric_limits<short>::max() &&
                              n >= std::numeric_limits<short>::min();
                     },
        "Arbitrary<short> should be in the expected range of shorts.")
    (_10M);

    property(prop_mean<int>( _10M, 0, 2, true),
        "Mean value of Arbitrary<int>(-1000, 1000) should be close enough to 0")
        .Rnd<0>(-1000, 1000)
    (_10M);

    property(prop_mean<unsigned int>( _10M, 500, 2),
        "Mean value of Arbitrary<unsigned int>(0, 1000) should be close enough to 500")
        .Rnd<0>(0, 1000)
    (_10M);

    property(prop_mean<int>( _10M, std::numeric_limits<int>::max() / 2, 1000000, true),
        "Mean value of Arbitrary<int>(0, std::numeric_limits<int>::max()) "
        "should be close to std::numeric_limits<int>::max() / 2")
        .Rnd<0>(0, std::numeric_limits<int>::max() - 1 )
    (_10M);

    property(prop_mean<double>( _10M, 0.0, 0.01, true),
        "Mean value of Arbitrary<double>() should be close to 0.0")
        .Rnd<0>(Arbitrary<double>())
    (_10M);

    property(prop_mean<double>( _10M, 0.5, 0.01, true),
        "Mean value of Arbitrary<double>() with x >= 0 should be close to 0.5")
        //<  Acceptor<double>([] (double x) { return x >= 0; }
        .Rnd<0>(Arbitrary<double>())
        .If(std::bind2nd(std::greater_equal<double>(), 0))
    (_10M, 0.7);

    property(prop_mean<float>(_10M, -13., 0.01, true),
        "Mean value of Arbitrary<float>(-28., 2.) should be close to -13.")
        .Rnd<0>(-28., 2.)
     (_10M);

    property(prop_mean<float>(_10M, -0.25, 0.01, true),
        "Mean value of Arbitrary<float>() whith x < 0.5 should be close to -0.25")
        .Rnd<0>(Arbitrary<float>())
        .If(std::bind2nd(std::less<double>(), 0.5))
     (_10M);

    property(prop_mean<float>(_10M, 3.7, 0.01, true),
        "Mean value of Arbitrary<float>(2.8, 4.6) should be close to 3.7")
        .Rnd<0>(2.8, 4.6)
     (_10M);
}

void test_oneof()
{
    std::vector<std::string> strings({"spam", "ham", "hello"});

    property([&strings] (const std::string & s) -> bool
                           { return std::find(strings.begin(),
                                              strings.end(), s)
                                    != strings.end();
                           },
        PROP"OneOf should return only values in its params list.")
        .One<0>(strings)
    (_10M);

    property([] (const std::vector<int> & v)
                    {   std::set<int> notseen(v.begin(), v.end());
                        property([&notseen] (int n)
                                { notseen.erase(n); return true; },
                            "Dummy property.", 0)
                            .One<0>(v)
                        (_1K);
                        return notseen.empty();
                    },
        PROP"OneOf, given enough time, should return all values in its params.")
        .If([](const std::vector<int> &v) { return v.size() > 0;} )
    (_1K);

    property(prop_mean<int>(_10M, 4, 0.01, true),
        "Mean value of Freq for values 1,5 with weights 1,3 should be 4.")
        .Frq<0>({{1, 1}, {5,3}})
    (_10M);
}

void test_vector()
{
    property([](const std::vector<int> &v)
                    { return v.size() >= 1 && v.size() <= 10;},
        "Vector size should be between 1 and 10.")
        .Rnd<0>(1,10)
    (_1K);

    property([](const std::vector<char> &v)
                    { return v.size() == 13 &&
                        std::accumulate(v.begin(), v.end(), true,
                            [](bool f, char b) { return f && (b >= 'a' &&
                                                    b <= 'z');});
                    },
        "Vector elements should be between 'a' and 'z'.")
        .Rnd<0>(Arbitrary<char>('a', 'z'), 13, 13)
    (_1K);
}



int main()
{
    std::cout<<"======================================"<<std::endl;
    std::cout<<"Running internal tests..."<<std::endl;
    test_arbit();
    test_arbit_bounded();
    test_oneof();
    test_vector();
    std::cout<<"===========tests end=================="<<std::endl;
}
