#ifndef _QCPPC_ACCEPTOR_H
#define _QCPPC_ACCEPTOR_H

namespace QuickCppCheck {

    namespace Detail {

template<typename T>
struct id {
    bool operator()(T & t) {
        return true;
    }
};

} // namespace Detail

template<typename T, size_t n = 0>
struct Acceptor
{
    typedef std::function<bool(T&)> FunType;
    FunType fun;
    Acceptor()
    {
        fun = Detail::id<T>();
    }
    Acceptor(FunType &&fun):fun(fun){};

    bool operator()(T &t)
    {
        return fun(t);
    }
};

} // namespace QuickCppCheck

#endif // _QCPPC_ACCEPTOR_H
