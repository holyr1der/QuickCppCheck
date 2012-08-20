#ifndef _QCPPC_PROPERTY_H
#define _QCPPC_PROPERTY_H

#include <iostream>
#include <functional>
#include <tuple>

#include "arbitrary.hpp"
#include "acceptor.hpp"
#include "printer.hpp"

namespace QuickCppCheck {
    namespace Detail {

template<std::size_t N>
struct apply_func {
    template<typename F, typename... Types, typename... Args>
    static bool apply(F & f, 
                    std::tuple<Arbitrary<Types>...> & arbs,
                    std::tuple<Acceptor<Types>...> & accs, 
                    std::tuple<Types...> & t, 
                    Args & ... args)
    {
        do {
            std::get<N-1>(t) = std::get<N-1>(arbs)();
        } while (!std::get<N-1>(accs)(std::get<N-1>(t)));

        return apply_func<N-1>::apply(f, arbs, accs, t, std::get<N-1>(t), args...);
    }
};

template<>
struct apply_func<0> {
    template<typename F, typename... Types, typename... Args>
    static bool apply(F & f, 
                    std::tuple<Arbitrary<Types>...> & arbs,
                    std::tuple<Acceptor<Types>...> & accs,
                    std::tuple<Types...> & t, 
                    Args & ... args)
    {
        return f(args...);
    }
};

} //namespace Detail

static Detail::ColoredString RED = Detail::ColoredString(Detail::RED_COLOR);
static Detail::ColoredString GREEN = Detail::ColoredString(Detail::GREEN_COLOR);
static Detail::ColoredString YELLOW = Detail::ColoredString(Detail::YELLOW_COLOR);

template<typename... Args>
class Property
{
private:
    std::tuple<typename std::decay<Args>::type...> data;
    std::tuple<Acceptor<typename std::decay<Args>::type>...> accs;
    std::tuple<Arbitrary<typename std::decay<Args>::type>...> arbs;

    typedef std::function<bool(Args...)> FunType;

    FunType fun;
    std::string name;
    int verbose;

    static const int MAX_TESTS = 1000000000; //that's one billion

    static bool initialized;

public:
    Property(FunType && fun, std::string && name = std::string("<unnamed>"), int verbose = 0):
        fun(fun), name(name), verbose(verbose)
    {}

    void operator()(int n = 1000)
    {
        bool ok = true;

        if (verbose) {
            std::cout<<std::endl<<"[--------start test--------]";
        }
        std::cout<<std::endl;
        std::cout<<"Property: "<<YELLOW(name)<<std::endl;

        unsigned int N = n < 0 ? MAX_TESTS : n;

        for (unsigned int i = 1;i <= N;++i) {
            if (!Detail::apply_func<sizeof...(Args)>::apply(fun, arbs, accs, data)) {
                std::cout<<RED("Failed.")<<" Falsifiable after "<<i<<" tests."<<std::endl;
                if (verbose) {
                    std::cout<<i<<": ";
                }
                print_tuple(data);
                ok = false;
                break;
            }
            if (verbose > 0) {
                std::cout<<i<<": ";
                print_tuple(data);
            }
        }

        if (ok) {
            std::cout<<GREEN("OK.")<<" Passed "<<N<<" tests."<<std::endl;
        }
        if (verbose) {
            std::cout<<"[--------end test------]"<<std::endl;
        }
    }

    template<typename T, size_t I>
    Property<Args...> & operator<(const Acceptor<T, I> & a)
    {
        std::get<I>(accs) = a;
        return *this;
    }

    template<typename T, size_t I, typename B = long>
    Property<Args...> & operator<=(const Arbitrary<T, I, B> & a)
    {
        std::get<I>(arbs) = a;
        return *this;
    }

    template<typename T, size_t I>
    Property<Args...> & operator<=(const Fixed<T, I> & a)
    {
        std::get<I>(arbs) = a;
        return *this;
    }

    template<typename T, size_t I, typename F = void>
    Property<Args...> & operator<=(const OneOf<T, I, F> & a)
    {
        std::get<I>(arbs) = a;
        return *this;
    }

};

} // namespace QuickCppCheck

#endif // _QCPPC_PROPERTY_H
