#ifndef QCPPC_ARBITRARY_H_
#define QCPPC_ARBITRARY_H_

#include <vector>
#include <map>
#include <random>
#include <functional>
#include <limits>
#include <cassert>
#include <type_traits>
#include <typeinfo>

namespace qcppc {

static const int MAX_LEN = 50;

template<typename T, typename Enable = void>
struct Arbitrary
{
    // This is just to allow the code to compile
    // even when Arbitrary is not specialized for type T.
    // We want this to compile because we expect the user to
    // supply later, at runtime, a function to generate random instances
    // of T. If he does not, however, we fail with a runtime error.
    //
    // NOTE: T has to be default constructible.
    // FIX: typeid does not return always a descriptive name.
    // FIX: Find a better way to handle this.
    T operator()(){
        std::cerr<<"Test error: ";
        std::cerr<<"No instance for Arbitrary<"<<typeid(T).name()<<">"<<
            " and no user supplied function/value."<<std::endl;
        exit(0);
        return T();
    }
};

// A wrapper for generators.
// It can hold any function with type T().
template<typename T>
struct Generator
{
    typedef std::function<T()> FunType;

    FunType fun;

    Generator():fun(Arbitrary<T>()) {};

    Generator(const FunType & fun):fun(fun) {};

    T operator()() {
        return fun();
    }

    Generator<T> & operator=(const FunType & fun) {
        this->fun = fun;
        return *this;
    }
};

// Initialize and seed a random engine, ready to be used by
// derived classes.
struct ArbitraryBase
{
    std::mt19937 engine;

    ArbitraryBase() {
        std::random_device rd;
        engine.seed(rd());
    }
};

// Functor that returns always the same value.
template<typename T>
struct Fixed
{
    T _val;

    Fixed(const T & v):_val(v) {};

    T operator()() {
        return _val;
    }
};

// Functor that returns randomly and with equal probability
// one of the elements of the vector it was initialized with.
template<typename T>
struct OneOf : ArbitraryBase
{
    std::vector<T> _vals;
    std::uniform_int_distribution<unsigned int> dist;

    OneOf(const std::vector<T> & v):_vals(v),dist(0, v.size() - 1) {
        assert(v.size() > 0);
    };

    T operator()() {
        return _vals[dist(engine)];
    }
};

// Weighted version of OneOf functor.
// Gets initialized with a map from values to weights and
// returns randomly a value with probability proportional to
// its weight.
template<typename T>
struct Freq : ArbitraryBase
{
    std::vector<T> _vals;
    std::discrete_distribution<unsigned int> dist;

    Freq(const std::map<T, double> & m){
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

// Arbitrary specialization for bool.
template<>
struct Arbitrary<bool> : ArbitraryBase
{
    std::uniform_int_distribution<int> dist;

    Arbitrary():dist(0,1) {}

    bool operator()() {
        return static_cast<bool>(dist(engine));
    }
};

// Arbitrary specialization for all integral types (signed and unsigned).
template<typename IntType>
struct Arbitrary<IntType, typename std::enable_if<
                    std::is_integral<IntType>::value>::type>
    : ArbitraryBase
{
    std::uniform_int_distribution<IntType> dist;

    Arbitrary(IntType low = std::numeric_limits<IntType>::min(),
              IntType high = std::numeric_limits<IntType>::max()):
            dist(low, high) {
        assert(low <= high);
    }

    IntType operator()() {
        return dist(engine);
    }
};

// Arbitrary specialization for floating point types.
template<typename RealType>
struct Arbitrary<RealType, typename std::enable_if<
                std::is_floating_point<RealType>::value>::type>
    : ArbitraryBase
{
    std::uniform_real_distribution<RealType> dist;

    Arbitrary(RealType low = -1.0, RealType high = 1.0):dist(low, high) {
       assert(low <= high);
    }

    RealType operator()() {
        return dist(engine);
    }
};

// Arbitrary specialization for strings.
template<typename charT>
struct Arbitrary<std::basic_string<charT>> : ArbitraryBase
{
    std::uniform_int_distribution<unsigned int> length;
    std::function<charT()> gen;

    // Creates a string generator with the default alphabet (ascii chars from 0x20 to 0xFF)
    // and size between low and high (inclusive).
    Arbitrary(int low = 0, int high = MAX_LEN):
        Arbitrary<std::basic_string<charT>>(Arbitrary<charT>(0x20, 0x7F), low, high) {}

    // Sets gen as the generator for chars, useful if we want to
    // restrict strings to a particular alphabet.
    Arbitrary(const std::function<charT()> & gen, int low = 0, int high = MAX_LEN):
        length(low, high), gen(gen) {
        assert(low <= high);
    }

    std::basic_string<charT> operator()() {
        unsigned int n = length(engine);
        std::basic_string<charT> res;
        for (unsigned int i = 0;i < n;++i) {
            res += gen();
        }
        return res;
    }
};

// Arbitrary specialization for std::vector.
template<typename T>
struct Arbitrary<std::vector<T>> : ArbitraryBase
{
    std::uniform_int_distribution<unsigned int> length;
    std::function<T()> gen;

    // Creates a generator that produces vector of length
    // between low and high (inclusive).
    Arbitrary(int low = 0, int high = MAX_LEN):
        Arbitrary<std::vector<T>>(Arbitrary<T>(), low, high) {}

    // Same as above overload, only use gen as the generator for
    // the contained type, instead of the default one.
    Arbitrary(const std::function<T()> & gen, int low = 0, int high = MAX_LEN):
        length(low, high), gen(gen) {
            assert(low <= high);
    }

    std::vector<T> operator()() {
        std::vector<T> v;
        unsigned int n = length(engine);
        for (unsigned int i = 0;i < n;++i) {
            v.push_back(gen());
        }
        return v;
    }
};

} // namespace qcppc


#endif // QCPPC_ARBITRARY_H_
