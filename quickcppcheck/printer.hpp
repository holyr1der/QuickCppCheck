#ifndef _QCPPC_PRINTER_H
#define _QCPPC_PRINTER_H

#include <tuple>
#include <sstream>

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

enum ColorCode {
    RED_COLOR = 1,
    GREEN_COLOR =2,
    YELLOW_COLOR = 3,
};

struct ColoredString {
    std::string pre;
    std::string suf;

    ColoredString(ColorCode cc) {
        std::stringstream ss;
        ss<<"\033[0;3"<<cc<<"m";
        pre = ss.str();
        suf = "\033[m";
    }
    std::string operator()(std::string &&in) {
        return pre + in + suf;
    }
    std::string operator()(std::string &in) {
        return pre + in + suf;
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

Detail::ColoredString RED = Detail::ColoredString(Detail::RED_COLOR);
Detail::ColoredString GREEN = Detail::ColoredString(Detail::GREEN_COLOR);
Detail::ColoredString YELLOW = Detail::ColoredString(Detail::YELLOW_COLOR);

} // namespace QuickCppCheck

#endif // _QCPPC_PRINTER_H
