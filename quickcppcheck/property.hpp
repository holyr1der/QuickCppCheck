#ifndef _QCPPC_PROPERTY_H
#define _QCPPC_PROPERTY_H

#include <iostream>
#include <functional>
#include <tuple>

#include "arbitrary.hpp"
#include "printer.hpp"

#define QCPP_STRINGIFY(x) #x
#define QCPP_TOSTRING(x) QCPP_STRINGIFY(x)
#define QCPP_AT __FILE__ ":" QCPP_TOSTRING(__LINE__)
#define PROP QCPP_AT ":"

namespace QuickCppCheck {
    namespace Detail {

template<std::size_t N>
struct apply_func {
    template<typename F, typename... Types, typename... Args>
    static typename F::result_type
            apply(F & f,
                    std::tuple<Types...> & data,
                    Args & ... args)
    {
        return apply_func<N-1>::apply(f, data, std::get<N-1>(data), args...);
    }
};

template<>
struct apply_func<0> {
    template<typename F, typename... Types, typename... Args>
    static typename F::result_type
            apply(F & f,
                    std::tuple<Types...> & data,
                    Args & ... args)
    {
        return f(args...);
    }
};

template<std::size_t N>
struct apply_func_individually {
    template<typename... Types>
    static void apply(std::tuple<Arbitrary<Types>...> & arbs,
                      std::tuple<Types...> & data)
    {
        std::get<N-1>(data) = std::get<N-1>(arbs)();
        apply_func_individually<N-1>::apply(arbs, data);
    }
};

template<>
struct apply_func_individually<0> {
    template<typename... Types>
    static void apply(std::tuple<Arbitrary<Types>...> & arbs,
                      std::tuple<Types...> & data)
    {}
};

template<size_t POS, typename T, typename...Args>
struct get_type_at
{
    typedef typename get_type_at<POS - 1, Args...>::type type;
};

template<typename T, typename...Args>
struct get_type_at<0, T, Args...>
{
    typedef typename std::decay<T>::type type;
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
    std::tuple<Arbitrary<typename std::decay<Args>::type>...> arbs;

    typedef std::function<bool(Args&...)> FunType;
    typedef std::function<bool(Args&...)> AcceptorType;

    FunType fun;
    AcceptorType acceptor;
    std::string name;
    int verbose;

    static const unsigned int MAX_TESTS = 1000;
    static const unsigned int MAX_FAILS = 1000;

    static bool initialized;

public:
    Property(const FunType & fun, const std::string & name = std::string("<unnamed>"), int verbose = 1):
        fun(fun), acceptor(NULL), name(name), verbose(verbose)
    {}

    void operator()(unsigned int ntests = MAX_TESTS, unsigned int nfails = MAX_FAILS)
    {
        bool ok = true;
        unsigned int fails = 0;

        if (verbose > 1) {
            std::cout<<std::endl<<"[--------start test--------]";
        }
        if (verbose > 0) {
            std::cout<<std::endl;
            std::cout<<"Property: "<<YELLOW(name)<<std::endl;
        }

        for (unsigned int i = 1; i <= ntests; ++i) {
            Detail::apply_func_individually<sizeof...(Args)>::apply(arbs, data);

            if (acceptor) {
                while (!Detail::apply_func<sizeof...(Args)>::apply(acceptor, data)) {
                    if (++fails == nfails) {
                        if (verbose > 0) {
                            std::cout<<RED("Arguments exhausted")<<" after "
                                <<fails<<" fails."<<std::endl;
                            return;
                        }
                    }
                    Detail::apply_func_individually<sizeof...(Args)>::apply(arbs, data);
                }
            }

            if (!Detail::apply_func<sizeof...(Args)>::apply(fun, data)) {
                if (verbose > 0) {
                    std::cout<<RED("Failed.")<<" Falsifiable after "<<i<<" tests."<<std::endl;
                    if (verbose > 1) {
                        std::cout<<i<<": ";
                    }
                    print_tuple(data);
                }
                ok = false;
                break;
            }
            if (verbose > 1) {
                std::cout<<i<<": ";
                print_tuple(data);
            }
        }

        if (ok) {
            if (verbose > 0) {
                std::cout<<GREEN("OK.")<<" Passed "<<ntests<<" tests."<<std::endl;
            }
        }
        if (verbose > 1) {
            std::cout<<"[--------end test------]"<<std::endl;
        }
    }

    template<size_t POS, PROVIDER P, typename T>
    Property<Args...> & _(T l, T h)
    {
        typedef typename Detail::get_type_at<POS, Args...>::type ValType;
        typedef typename Detail::select<P, ValType, T>::RType ArbitraryType;
        std::get<POS>(arbs) = ArbitraryType(l, h);
        return *this;
    }

    template<size_t POS, PROVIDER P, typename T>
    Property<Args...> & _(const T &t, typename std::enable_if<P==RAND>::type* = 0)
    {
        typedef typename Detail::get_type_at<POS, Args...>::type ValType;
        typedef typename Detail::select<P, ValType>::RType ArbitraryType;
        std::get<POS>(arbs) = ArbitraryType(t);
        return *this;
    }

    template<size_t POS, PROVIDER P, typename ValType =
                typename Detail::get_type_at<POS, Args...>::type>
    Property<Args...> & _(const ValType &v,
            typename std::enable_if<P==FIX>::type* = 0)
    {
        typedef typename Detail::select<P, ValType>::RType ArbitraryType;
        std::get<POS>(arbs) = ArbitraryType(v);
        return *this;
    }

    template<size_t POS, PROVIDER P, typename ValType =
                typename Detail::get_type_at<POS, Args...>::type>
    Property<Args...> & _(const std::vector<ValType> &v,
            typename std::enable_if<P==ONE>::type* = 0)
    {
        typedef typename Detail::select<P, ValType>::RType ArbitraryType;
        std::get<POS>(arbs) = ArbitraryType(v);
        return *this;
    }

    template<size_t POS, PROVIDER P, typename ValType =
                typename Detail::get_type_at<POS, Args...>::type>
    Property<Args...> & _(const std::map<ValType, double> &v,
            typename std::enable_if<P==FREQ>::type* = 0)
    {
        typedef typename Detail::select<P, ValType>::RType ArbitraryType;
        std::get<POS>(arbs) = ArbitraryType(v);
        return *this;
    }

    Property<Args...> & operator|(const AcceptorType & acceptor)
    {
        this->acceptor = acceptor;
        return *this;
    }
};

} // namespace QuickCppCheck

#endif // _QCPPC_PROPERTY_H
