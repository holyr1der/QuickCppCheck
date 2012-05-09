#ifndef _QCPPC_ARBITRARY_H
#define _QCPPC_ARBITRARY_H

#include <vector>
#include <random>
#include <limits>
#include <cassert>

namespace QuickCppCheck {

    namespace Detail {

static const int MAX_LEN = 50;

std::mt19937 engine;

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

    Arbitrary(long low, long high) {
        fun = Detail::ArbitraryImpl<T>(low, high);
    }

    T operator()() {
        return fun();
    }

    template<size_t m>
    Arbitrary<T> & operator=(const Arbitrary<T, m> &a) {
        this->fun = a.fun;
        return *this;
    }

};

namespace Detail {

enum STATE {
    LOW,
    HIGH,
    RAND,
};

template<typename IntType>
struct ArbitraryIntBase {
    STATE state;
    std::uniform_int_distribution<IntType> dist;

    ArbitraryIntBase(IntType low = std::numeric_limits<IntType>::min(),
                     IntType high = std::numeric_limits<IntType>::max()):
        state(LOW), dist(low, high) {
        assert(low <= high);
    }
    IntType operator()() {
        switch (state) {
            case LOW: 
                state = HIGH; 
                return dist.min();
            case HIGH: 
                state = RAND; 
                return dist.max();
            case RAND:
            default: 
                return dist(engine);
        }
    }
};


template<>
struct ArbitraryImpl<unsigned char> : ArbitraryIntBase<unsigned char>{
    template<typename...Args>
    ArbitraryImpl(Args&&...args): ArbitraryIntBase<unsigned char>(args...) {}
};

template<>
struct ArbitraryImpl<char> : ArbitraryIntBase<char>{
    template<typename...Args>
    ArbitraryImpl(Args&&...args): ArbitraryIntBase<char>(args...) {}
};

template<>
struct ArbitraryImpl<unsigned short> : ArbitraryIntBase<unsigned short> {
    template<typename...Args>
    ArbitraryImpl(Args&&...args): ArbitraryIntBase<unsigned short>(args...) {}
};

template<>
struct ArbitraryImpl<short> : ArbitraryIntBase<short>{
    template<typename...Args>
    ArbitraryImpl(Args&&...args): ArbitraryIntBase<short>(args...) {}
};

template<>
struct ArbitraryImpl<unsigned int> : ArbitraryIntBase<unsigned int>{
    template<typename...Args>
    ArbitraryImpl(Args&&...args): ArbitraryIntBase<unsigned int>(args...) {}
};

template<>
struct ArbitraryImpl<int> : ArbitraryIntBase<int>{
    template<typename...Args>
    ArbitraryImpl(Args&&...args): ArbitraryIntBase<int>(args...) {}
};

template<>
struct ArbitraryImpl<unsigned long> : ArbitraryIntBase<unsigned long>{
    template<typename...Args>
    ArbitraryImpl(Args&&...args): ArbitraryIntBase<unsigned long>(args...) {}
};

template<>
struct ArbitraryImpl<long> : ArbitraryIntBase<long>{
    template<typename...Args>
    ArbitraryImpl(Args&&...args): ArbitraryIntBase<long>(args...) {}
};

template<typename RealType>
struct ArbitraryRealBase {
    STATE state;
    std::uniform_real_distribution<RealType> dist;

    ArbitraryRealBase(RealType low = -1.0, RealType high = 1.0):
        state(LOW), dist(low, high) {
            assert(low <= high);
        }
    RealType operator()() {
        switch (state) {
            case LOW: 
                state = HIGH; 
                return dist.min();
            case HIGH:
                state = RAND;
                return dist.max();
            case RAND:
            default:
                return dist(engine);
        }
    }
};

template<>
struct ArbitraryImpl<float> : ArbitraryRealBase<float>{
    template<typename...Args>
    ArbitraryImpl(Args&&...args):ArbitraryRealBase<float>(args...){}
};

template<>
struct ArbitraryImpl<double> : ArbitraryRealBase<double>{
    template<typename...Args>
    ArbitraryImpl(Args&&...args):ArbitraryRealBase<double>(args...){}
};

template<>
struct ArbitraryImpl<std::string> {
    std::uniform_int_distribution<int> length; 
    std::uniform_int_distribution<int> chars; 

    ArbitraryImpl(int low = 0, int high = MAX_LEN):length(low, high), chars(32, 126) {}

    std::string operator()() {
        unsigned int n = length(engine);
        std::string res;
        for (unsigned int i = 0;i < n;++i) {
            res += chars(engine);
        }
        return res;
    }
};

template<typename T>
struct ArbitraryImpl<std::vector<T>> {
    std::uniform_int_distribution<int> length; 
    Arbitrary<T> arb; 

    ArbitraryImpl(int low = 0, int high = MAX_LEN):length(low, high) {}

    std::vector<T> operator()() {
        std::vector<T> v;
        unsigned int n = length(engine);
        for (unsigned int i = 0;i < n;++i) {
            v.push_back(arb());
        }
        return v;
    }
};

} // namespace Detail
} // namespace QuickCppCheck


#endif // _QCPPC_ARBITRARY_H
