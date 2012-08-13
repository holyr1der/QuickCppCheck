#include <iostream>

#include "../quickcppcheck/property.hpp"

#define _1M   1000000
#define _10M  10000000
#define _100M 100000000
#define _1G   1000000000

using namespace QuickCppCheck;

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

    (Property<int>(prop_mean<int>( _10M, 0, 1000000, true),
            "Mean value of Arbitrary<int> should be close to 0")
        <= Arbitrary<int>(std::numeric_limits<int>::min(), std::numeric_limits<int>::max())
    )
    (_10M);

    /* 
    (Property<long>(prop_mean<long>( _10M, 0, 10000, true),
            "Mean value of Arbitrary<long> should be close to 0")
        )
    (_10M);
    */
    Property<short>(prop_mean<short>( _10M, 0, 100, true),
            "Mean value of Arbitrary<short> should be close to 0")
    (_10M);

    Property<char>(prop_mean<char>( _10M, 0, 1, true),
            "Mean value of Arbitrary<char> should be close to 0")
    (_10M);

}

void test_arbit_bounded()
{
    (Property<int>([](int a) { return a <= 100 && a >= -100;},
        "Arbitrary<int>(-100, 100) should return between -100 and 100 inclusive",
        0)
        <= Arbitrary<int>(-100, 100))
    (_10M);

    (Property<unsigned int>([](unsigned int a) { return a <= 1000 && a >= 0;},
        "Arbitrary<unsigned int>(0, 1000) should return between 0 and 1000 inclusive",
        0)
        <= Arbitrary<unsigned int>(0, 1000))
    (_10M);

    (Property<short>([] (short n) 
                     { return n <= std::numeric_limits<short>::max() &&
                              n >= std::numeric_limits<short>::min();
                     },
        "Arbitrary<short> should be in the expected range of shorts.",
        0))
    (_10M);

    (Property<int>(prop_mean<int>( _10M, 0, 2, true),
        "Mean value of Arbitrary<int>(-1000, 1000) should be close enough to 0",
        0)
        <= Arbitrary<int>(-1000, 1000))
    (_10M);

    (Property<unsigned int>(prop_mean<unsigned int>( _10M, 500, 2),
        "Mean value of Arbitrary<unsigned int>(0, 1000) should be close enough to 500",
        0)
        <= Arbitrary<unsigned int>(0, 1000))
    (_10M);

    (Property<int>(prop_mean<int>( _10M, std::numeric_limits<int>::max() / 2, 1000000, true),
        "Mean value of Arbitrary<int>(0, std::numeric_limits<int>::max()) should be close to std::numeric_limits<int>::max() / 2",
        0)
        <= Arbitrary<int>(0, std::numeric_limits<int>::max() - 1 ))
    (_10M);

    (Property<double>(prop_mean<double>( _10M, 0.0, 0.01, true),
        "Mean value of Arbitrary<double>() should be close to 0.0",
        0)
        <= Arbitrary<double>())
    (_10M);

    (Property<double>(prop_mean<double>( _10M, 0.5, 0.01, true),
        "Mean value of Arbitrary<double>() with x >= 0 should be close to 0.5",
        0)
        <  Acceptor<double>([] (double x) { return x >= 0; })
        <= Arbitrary<double>())
    (_10M);

    (Property<float>(prop_mean<float>(_10M, -13., 0.01, true),
        "Mean value of Arbitrary<float>(-28., 2.) should be close to -13.")
        <=Arbitrary<float>(-28., 2.))
     (_10M);

    (Property<float>(prop_mean<float>(_10M, -0.25, 0.01, true),
        "Mean value of Arbitrary<float>() whith x < 0.5 should be close to -0.25")
        < Acceptor<float>(std::bind2nd(std::less<float>(), 0.5))
        <=Arbitrary<float>())
     (_10M);

    (Property<float>(prop_mean<float>(_10M, 3.7, 0.01, true),
        "Mean value of Arbitrary<float>(2.8, 4.6) should be close to 3.7")
        <= Arbitrary<float, 0, float>(2.8, 4.6))
     (_10M);
}



int main()
{
    std::cout<<"======================================"<<std::endl;
    std::cout<<"Running internal tests..."<<std::endl;
    test_arbit();
    test_arbit_bounded();
    std::cout<<"===========tests end=================="<<std::endl;
}
