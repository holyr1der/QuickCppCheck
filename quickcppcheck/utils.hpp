// Utilities for qcppc

#ifndef QCPPC_UTILS_H_
#define QCPPC_UTILS_H_

namespace qcppc {
namespace utils {

//---------------------------------------------------------------
// Applies a function on a tuple.
// Extract one element from the tuple each time and apply
// recursively for the rest.
template<std::size_t N>
struct apply_func {
    template<typename F, typename... Types, typename... Args>
    static typename F::result_type
            apply(F & f,
                  std::tuple<Types...> & data,
                  Args&... args)
    {
        return apply_func<N-1>::apply(f, data, std::get<N-1>(data), args...);
    }
};

// Base case; no more elements to extract, just call the function.
template<>
struct apply_func<0> {
    template<typename F, typename... Types, typename... Args>
    static typename F::result_type
            apply(F & f,
                  std::tuple<Types...> & data,
                  Args&... args)
    {
        return f(args...);
    }
};

//--------------------------------------------------------------
// Gets a tuple of functions and a tuple of arguments and applies each
// function to the corresponding argument.
template<std::size_t N>
struct apply_func_individually {
    template<typename Funs, typename Data>
    static void apply(Funs & funs, Data & data)
    {
        std::get<N-1>(data) = std::get<N-1>(funs)();
        apply_func_individually<N-1>::apply(funs, data);
    }
};

// Base case; nothing to do, just stop the recursion.
template<>
struct apply_func_individually<0> {
    template<typename Funs, typename Data>
    static void apply(Funs & funs, Data & data)
    {}
};

//--------------------------------------------------------------
//Find the type at POS position in argument pack Args.
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

//-------------------------------------------------------------
// Function traits.
//
// Get compile time information about function objects.
//
template<typename T>
struct function_traits
{
    typedef typename function_traits<decltype(&T::operator())>::return_type return_type;
    typedef typename function_traits<decltype(&T::operator())>::arg_types arg_types;
};

// Member functions.
template<typename R, typename C, typename...Args>
struct function_traits<R (C::*)(Args...)>
{
    typedef R return_type;
    typedef std::tuple<Args...> arg_types;
};

// const member functions.
template<typename R, typename C, typename...Args>
struct function_traits<R (C::*)(Args...) const>
{
    typedef R return_type;
    typedef std::tuple<Args...> arg_types;
};

// Function pointers.
template<typename R, typename...Args>
struct function_traits<R (*)(Args...)>
{
    typedef R return_type;
    typedef std::tuple<Args...> arg_types;
};

// Normal functions.
template<typename R, typename...Args>
struct function_traits<R (Args...)>
{
    typedef R return_type;
    typedef std::tuple<Args...> arg_types;
};

// We can't typedef variadic argument packs, so, in function traits above,
// we use std::tuple to store them. Struct Unpack unpacks a variadic
// argument pack from an std::tuple and instantiates Target with that.
template<typename Pack, template<typename...> class Target>
struct Unpack {};

template<typename...Args, template<typename...> class Target>
struct Unpack<std::tuple<Args...>, Target>
{
    typedef Target<Args...> type;
};

template<typename T>
struct is_printable
{
    template<typename U> static char test(char(*)[sizeof(*((std::ostream*)0)<<
                *(U*)0)]);
    template<typename U> static int test(...);

    enum { value = sizeof(test<T>(0)) == 1 };
};

} // namespace utils
} // namespace qcppc

#endif // QCPPC_UTILS_H_
