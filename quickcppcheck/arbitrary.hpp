#ifndef _QCPPC_ARBITRARY_H
#define _QCPPC_ARBITRARY_H

#include <vector>
#include <set>
#include <cstdlib>
#include <climits>

namespace QuickCppCheck {

    namespace Detail {

template<typename T>
struct ArbitraryImpl;

} // namespace Detail

template<typename T, size_t n = 0>
struct Arbitrary
{
    typedef std::function<T()> FunType;
    FunType fun;

    Arbitrary() {
        fun = Detail::ArbitraryImpl<T>();
    }

    Arbitrary(FunType &&fun):fun(fun) {};

    T operator()() {
        return fun();
    }

    template<size_t m>
    Arbitrary<T> & operator=(Arbitrary<T, m> &&a) {
        this->fun = a.fun;
        return *this;
    }
};

namespace Detail {

#define RANDOM_SIGN(X)  ((((rand() % 2)<<1) - 1) * (X))

template<>
struct ArbitraryImpl<unsigned short> {
    unsigned short operator()() {
        return rand() % USHRT_MAX;
    }
};

template<>
struct ArbitraryImpl<short> {
    short operator()() {
        return RANDOM_SIGN(rand() % SHRT_MAX);
    }
};

template<>
struct ArbitraryImpl<unsigned int> {
    unsigned int operator()() {
        return rand() % UINT_MAX;
    }
};

template<>
struct ArbitraryImpl<int> {
    int operator()() {
        return RANDOM_SIGN(rand() % INT_MAX);
    }
};

template<>
struct ArbitraryImpl<unsigned long> {
    unsigned long operator()() {
        return rand() % ULONG_MAX;
    }
};

template<>
struct ArbitraryImpl<long> {
    long operator()() {
        return RANDOM_SIGN(rand() % LONG_MAX);
    }
};

template<>
struct ArbitraryImpl<float> {
    float operator()() {
        return RANDOM_SIGN((float)rand() / RAND_MAX);
    }
};

template<>
struct ArbitraryImpl<double> {
    double operator()() {
        return RANDOM_SIGN((double)rand() / RAND_MAX);
    }
};

template<>
struct ArbitraryImpl<unsigned char> {
    unsigned char operator()() {
        return rand() % 256;
    }
};

template<>
struct ArbitraryImpl<char> {
    char operator()() {
        return RANDOM_SIGN(rand() % CHAR_MAX);
    }
};

template<>
struct ArbitraryImpl<std::string> {
    std::string operator()() {
        int n = ArbitraryImpl<int>()();
        std::string res;
        for (int i = 0;i < n % 50;++i) {
            res += rand() % 96 + 32;
        }
        return res;
    }
};

template<typename T>
struct ArbitraryImpl<std::vector<T>> {
    std::vector<T> operator()() {
        std::vector<T> v;
        int n = ArbitraryImpl<int>()();
        for (int i = 0;i < n % 20;++i) {
            v.push_back(ArbitraryImpl<T>()());
        }
        return v;
    }
};

template<typename T>
struct ArbitraryImpl<std::set<T>> {
    std::set<T> operator()() {
        std::set<T> s;
        int n = ArbitraryImpl<int>()();
        for (int i = 0;i < n % 20;++i) {
            s.insert(ArbitraryImpl<T>()());
        }
        return s;
    }
};

} // namespace Detail
} // namespace QuickCppCheck

#endif // _QCPPC_ARBITRARY_H
