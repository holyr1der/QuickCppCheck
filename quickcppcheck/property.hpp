#ifndef QCPPC_PROPERTY_H_
#define QCPPC_PROPERTY_H_

#include <iostream>
#include <functional>
#include <tuple>

#include "utils.hpp"
#include "generator.hpp"
#include "printer.hpp"
#include "classifier.hpp"

#define QCPP_STRINGIFY(x) #x
#define QCPP_TOSTRING(x) QCPP_STRINGIFY(x)
#define QCPP_AT __FILE__ ":" QCPP_TOSTRING(__LINE__)
#define PROP QCPP_AT ":"

namespace qcppc {

using namespace utils;
using detail::operator<<;

#ifdef HAS_COLOR
static detail::ColoredString MAKE_RED = detail::ColoredString(detail::RED_COLOR);
static detail::ColoredString MAKE_GREEN = detail::ColoredString(detail::GREEN_COLOR);
static detail::ColoredString MAKE_YELLOW = detail::ColoredString(detail::YELLOW_COLOR);
#else
# define MAKE_RED(X) (X)
# define MAKE_GREEN(X) (X)
# define MAKE_YELLOW(X) (X)
#endif

// An object of class Property contains the execution environment of a property.
// That is, the property function itself and the data generators and constraints
// on these data.
template<typename... Args>
class Property
{
private:
    // The data to supply to property function.
    std::tuple<typename std::decay<Args>::type...> data;

    // Generators for data. Each generator provides a value of type T,
    // where T is the type of the corresponding position in member data.
    std::tuple<Generator<typename std::decay<Args>::type>...> generators;

    typedef std::function<bool(Args&...)> FunType;
    typedef std::function<bool(Args&...)> AcceptorType;
    typedef typename Classifier<Args...>::classifier_type ClassifierType;

    // The property function.
    FunType prop_fun;

    // The acceptor function.
    AcceptorType acceptor;

    Classifier<typename std::decay<Args>::type...> classifier;

    // Description of the property.
    std::string name;

    // Output verbosity level.
    int verbose;

    // Max number of tests to run, if user does not specify.
    static constexpr unsigned int MAX_TESTS = 1000;

    // The number of discarded tests before giving up.
    static constexpr float DISCARDED_RATIO = 0.5;

    void print_classifier_result() {
        if (classifier) {
            classifier.print_results();
        }
    }

public:
    // Constructs a Property object around the function prop_fun.
    //
    // Arguments:
    //
    // prop_fun: can be of any type of callable object as long as
    //  its return type is bool.
    //
    // name: a string describing the property.
    //
    // verbose: the verbosity level of the output. It can take the following values:
    //  0: no output
    //  1: (default) print only the name of the property and the result of the test.
    //  2: print also all the generated test cases.
    Property(const FunType & prop_fun, const std::string & name =
            std::string("<unnamed>"), int verbose = 1):
        prop_fun(prop_fun), acceptor(nullptr), classifier(nullptr),
        name(name), verbose(verbose)
    {}

    // Executes the tests until we reach ntests successful tests
    // or the test fails or we reach discarded_ratio * ntests discarded tests.
    bool operator()(unsigned int ntests = MAX_TESTS,
            float discarded_ratio = DISCARDED_RATIO)
    {
        bool ok = true;
        unsigned int discarded = 0;
        unsigned int max_discarded = discarded_ratio * ntests;

        if (verbose > 1) {
            std::cout<<"[--------start test--------]"<<std::endl;
        }
        if (verbose > 0) {
            std::cout<<"Property: "<<MAKE_YELLOW(name)<<std::endl;
        }

        for (unsigned int i = 1; i <= ntests; ++i) {
            apply_func_individually<sizeof...(Args)>::apply(generators, data);

            if (acceptor) {
                while (!apply_func<sizeof...(Args)>::apply(acceptor, data)) {
                    if (++discarded == max_discarded) {
                        if (verbose > 0) {
                            std::cout<<MAKE_RED("!!! Arguments exhausted")<<" after "
                                <<i<<" tests and "
                                <<discarded<<" discarded inputs."<<std::endl;
                            print_classifier_result();
                            std::cout<<std::endl;
                            return false;
                        }
                    }
                    apply_func_individually<sizeof...(Args)>::apply(generators, data);
                }
            }

            if (classifier) {
                classifier.classify(data);
            }
            if (verbose > 1) {
                std::cout<<i<<": ";
                std::cout<<data;
                std::cout.flush();
            }
            if (!apply_func<sizeof...(Args)>::apply(prop_fun, data)) {
                if (verbose > 0) {
                    std::cout<<MAKE_RED("*** Falsifiable,")<<" after "<<i<<" tests."<<std::endl;
                    if (verbose > 1) {
                        std::cout<<i<<": ";
                    }
                    std::cout<<data;
                    print_classifier_result();
                }
                ok = false;
                break;
            }
        }

        if (ok) {
            if (verbose > 0) {
                std::cout<<MAKE_GREEN("+++ OK,")<<" passed "<<ntests<<" tests."<<std::endl;
                print_classifier_result();
            }
        }
        if (verbose > 1) {
            std::cout<<"[--------end test------]"<<std::endl;
        }
        if (verbose > 0) {
            std::cout<<std::endl;
        }
        return ok;
    }

    // Sets the function fun as the generator for the argument at posistion POS.
    template<size_t POS, typename ValType =
                typename get_type_at<POS, Args...>::type,
            typename T = std::function<ValType()>>
    Property<Args...> & Rnd(const T & fun)
    {
        std::get<POS>(generators) = Generator<ValType>(fun);
        return *this;
    }

    // Creates an Arbitrary generator with the given parameters
    // and assigns it to argument at position POS.
    template<size_t POS, typename...Params, typename Enable =
            typename std::enable_if<(sizeof...(Params) > 1)>::type>
    Property<Args...> & Rnd(Params...params)
    {
        typedef typename get_type_at<POS, Args...>::type ValType;
        std::get<POS>(generators) = Arbitrary<ValType>(std::forward<Params>(params)...);
        return *this;
    }

    // Creates a Fixed generator with the given value and assigns it
    // to argument position POS.
    template<size_t POS, typename ValType =
                typename get_type_at<POS, Args...>::type>
    Property<Args...> & Fix(const ValType &v)
    {
        std::get<POS>(generators) = Fixed<ValType>(v);
        return *this;
    }

    // Creates an OneOf generator and assigns it to argument position POS.
    template<size_t POS, typename ValType =
                typename get_type_at<POS, Args...>::type>
    Property<Args...> & One(const std::vector<ValType> &v)
    {
        std::get<POS>(generators) = OneOf<ValType>(v);
        return *this;
    }

    // Creates a Freq generator and assigns it to argument position POS.
    template<size_t POS, typename ValType =
                typename get_type_at<POS, Args...>::type>
    Property<Args...> & Frq(const std::map<ValType, double> &v)
    {
        std::get<POS>(generators) = Freq<ValType>(v);
        return *this;
    }

    // Sets the acceptor function.
    Property<Args...> & If(const AcceptorType & acceptor)
    {
        this->acceptor = acceptor;
        return *this;
    }

    // Sets the classifier function. It should have type
    // std::string (Args...)
    Property<Args...> & Classify(const ClassifierType & classifier_fun)
    {
        classifier = Classifier<typename std::decay<Args>::type...>(classifier_fun);
        return *this;
    }

};

// Helper function to create objects of Property class.
// Extracts the types of the arguments of the property function
// and uses them to instantiate template class Property.
template<typename T, typename...Params,
    typename Property = typename Unpack<
            typename function_traits<T>::arg_types, Property
        >::type>
typename std::enable_if<std::is_same<
        typename function_traits<T>::return_type, bool
      >::value, Property
    >::type
property(const T & t, Params...params)
{
    return Property(t, std::forward<Params>(params)...);
}

} // namespace qcppc

#endif // QCPPC_PROPERTY_H_
