#ifndef _QCPPC_ARBITRARY_H
#define _QCPPC_ARBITRARY_H

#include <vector>
#include <map>
#include <random>
#include <limits>
#include <cassert>
#include <type_traits>

namespace QuickCppCheck {

    namespace Detail {

static const int MAX_LEN = 50;

struct ArbitraryImplBase {
    std::mt19937 engine;

    ArbitraryImplBase() {
        std::random_device rd;
        engine.seed(rd());
    }
};

template<typename T, typename Enable = void>
struct ArbitraryImpl {
    T operator()() {
        T t;
        return t;
    }
};

} // namespace Detail

template<typename T, size_t n = 0>
struct Fixed
{
    T _val;

    Fixed(const T & v):_val(v) {};

    T operator()() {
        return _val;
    }
};

struct FREQ {};

template<typename T, size_t n = 0, typename F = void>
struct OneOf;

template<typename T, size_t n>
struct OneOf<T, n> : Detail::ArbitraryImplBase
{
    std::vector<T> _vals;
    std::uniform_int_distribution<unsigned int> dist;

    OneOf(const std::vector<T> & v):
        ArbitraryImplBase(),_vals(v),dist(0, v.size() - 1) {
        assert(v.size() > 0);
    };

    T operator()() {
        return _vals[dist(engine)];
    }
};

template<typename T, size_t n>
struct OneOf<T, n, FREQ> : Detail::ArbitraryImplBase
{
    std::vector<T> _vals;
    std::discrete_distribution<unsigned int> dist;

    OneOf(const std::map<T, double> & m):ArbitraryImplBase() {
        assert(m.size() > 0);

        std::vector<double> _freqs;
        for(auto it = m.begin(); it !=  m.end(); ++it) {
            _vals.push_back(it->first);
            _freqs.push_back(it->second);
        };
        dist = std::discrete_distribution<unsigned int>(_freqs.begin(), _freqs.end());
    }

    T operator()() {
        return _vals[dist(engine)];
    }
};

template<typename T, size_t n = 0, typename B = long>
struct Arbitrary
{
    typedef std::function<T()> FunType;
    FunType fun;

    Arbitrary():fun(Detail::ArbitraryImpl<T>()) {};

    Arbitrary(const FunType & fun):fun(fun) {};

    Arbitrary(B low, B high):fun(Detail::ArbitraryImpl<T>(low, high)) {};

    T operator()() {
        return fun();
    }

    Arbitrary<T> & operator=(const FunType & fun) {
        this->fun = fun;
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
struct ArbitraryImpl<IntType, typename std::enable_if<std::is_integral<IntType>::value>::type> : ArbitraryImplBase {
    STATE state;
    std::uniform_int_distribution<IntType> dist;

    ArbitraryImpl(IntType low = std::numeric_limits<IntType>::min(),
                  IntType high = std::numeric_limits<IntType>::max()):
        ArbitraryImplBase(), state(LOW), dist(low, high) {
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

template<typename RealType>
struct ArbitraryImpl<RealType, typename std::enable_if<std::is_floating_point<RealType>::value>::type> : ArbitraryImplBase {
    STATE state;
    std::uniform_real_distribution<RealType> dist;

    ArbitraryImpl(RealType low = -1.0, RealType high = 1.0):
        ArbitraryImplBase(), state(LOW), dist(low, high) {
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
struct ArbitraryImpl<std::string> : ArbitraryImplBase {
    std::uniform_int_distribution<int> length;
    std::uniform_int_distribution<int> chars;

    ArbitraryImpl(int low = 0, int high = MAX_LEN):
        ArbitraryImplBase(), length(low, high), chars(32, 126) {}

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
struct ArbitraryImpl<std::vector<T>> : ArbitraryImplBase {
    std::uniform_int_distribution<int> length;
    Arbitrary<T> arb;

    ArbitraryImpl(int low = 0, int high = MAX_LEN):
        ArbitraryImplBase(), length(low, high) {}

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
