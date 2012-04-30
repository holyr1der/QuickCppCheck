#ifndef _QCPPC_ARBITRARY_H
#define _QCPPC_ARBITRARY_H

#include <vector>
#include <set>
#include <cstdlib>

namespace QuickCppCheck {

    namespace Detail {

template<typename T>
struct ArbitraryImpl;

} // namespace Detail

template<typename T, size_t n = 0>
struct Arbitrary
{
    //typedef T (*FunType)();
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

template<>
struct ArbitraryImpl<int> {
    int operator()() {
        return rand() % 1000;
    }
};

template<>
struct ArbitraryImpl<double> {
    double operator()() {
        return (double)ArbitraryImpl<int>()() / 1000;
    }
};

template<>
struct ArbitraryImpl<char> {
    char operator()() {
        return rand() % 256;
    }
};

template<>
struct ArbitraryImpl<std::string> {
    std::string operator()() {
        int n = ArbitraryImpl<int>()();
        std::string res;
        for (int i = 0;i < n % 50;++i) {
            res += ArbitraryImpl<int>()() % 96 + 32;
        }
        return res;
    }
};

template<typename T>
struct ArbitraryImpl<std::vector<T>> {
    std::vector<T> operator()() {
        std::vector<T> v;
        int n = ArbitraryImpl<int>()();
        //v.resize((unsigned) n);
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
