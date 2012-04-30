#ifndef _QCPPC_PRINTER_H
#define _QCPPC_PRINTER_H

#include <tuple>

namespace QuickCppCheck {
    namespace Detail {

template<size_t N>
struct tuple_printer_impl {
    template<typename Fun, typename... Types>
    void operator()(Fun fun, std::tuple<Types...> &tup) {
        tuple_printer_impl<N-1>()(fun, tup);
        fun(std::get<N - 1>(tup), sizeof...(Types) - N);
    }
};

template<>
struct tuple_printer_impl<0> {
    template<typename Fun, typename... Types>
    void operator()(Fun fun, std::tuple<Types...> &tup) {
    }
};

struct tuple_printer {
    std::ostream & out;
    std::string delimiter;
    tuple_printer(std::ostream &out, std::string delimiter):out(out),delimiter(delimiter){}
    template<typename T>
    void operator()(T &t, size_t n) {
        out<<t; if (n) out<<delimiter;
    }
};

} // namespace Detail

template<typename... Types>
void print_tuple(std::tuple<Types...> &tup, 
                 std::ostream &out = std::cout,
                 std::string delimiter = ", ") 
{
    out<<"[";
    Detail::tuple_printer_impl<sizeof...(Types)>()(Detail::tuple_printer(out, delimiter), tup);
    out<<"]"<<std::endl;
}

} // namespace QuickCppCheck

#endif // _QCPPC_PRINTER_H
