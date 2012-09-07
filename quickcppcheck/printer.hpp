#ifndef QCPPC_PRINTER_H_
#define QCPPC_PRINTER_H_

#include <tuple>
#include <sstream>

namespace qcppc {
namespace detail {

template<typename T>
std::ostream& operator<<(std::ostream& out, const std::vector<T> & v)
{
    out<<"V<";
    for (size_t i = 0;i < v.size();++i) {
        out<<v[i]<<", ";
    }
    out<<">";
    return out;
}

template<size_t N>
struct tuple_printer_impl {
    template<typename Fun, typename... Types>
    void operator()(const Fun & fun, const std::tuple<Types...> & tup) {
        tuple_printer_impl<N-1>()(fun, tup);
        fun(std::get<N - 1>(tup), sizeof...(Types) - N);
    }
};

template<>
struct tuple_printer_impl<0> {
    template<typename Fun, typename... Types>
    void operator()(const Fun & fun, const std::tuple<Types...> & tup) {
    }
};

struct tuple_printer {
    std::ostream & out;
    std::string delimiter;
    tuple_printer(std::ostream &out, std::string delimiter):out(out),delimiter(delimiter){}
    template<typename T>
    void operator()(const T & t, size_t n) const {
        using detail::operator<<;
        out<<t; if (n) out<<delimiter;
    }
};

enum ColorCode {
    RED_COLOR = 1,
    GREEN_COLOR = 2,
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

    std::string operator()(const std::string &in) {
        return pre + in + suf;
    }
};

template<typename... Types>
void print_tuple(const std::tuple<Types...> & tup, 
                 std::ostream &out = std::cout,
                 std::string delimiter = ", ") 
{
    out<<"[";
    tuple_printer_impl<sizeof...(Types)>()(tuple_printer(out, delimiter), tup);
    out<<"]"<<std::endl;
}

template<typename...Types>
std::ostream & operator<<(std::ostream & out, const std::tuple<Types...> & tup)
{
    print_tuple(tup, out);
    return out;
}
} // namespace detail

} // namespace qcppc

#endif // QCPPC_PRINTER_H_
