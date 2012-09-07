#ifndef QCPPC_CLASSIFY_H_
#define QCPPC_CLASSIFY_H_

#include <string>
#include <map>
#include <tuple>
#include <algorithm>
#include <iomanip>
#include <cmath>

#include "utils.hpp"

namespace qcppc {

template<typename...Args>
class Classifier
{
public:
    typedef std::function<std::string(Args&...)> classifier_type;
private:
    classifier_type m_classifier_fun;
    size_t m_total;
    std::map<std::string, size_t> m_classes;

public:

    Classifier():Classifier(nullptr) {}

    Classifier(const classifier_type & classifier_fun):
        m_classifier_fun(classifier_fun), m_total(0)
    {}

    Classifier<Args...> & operator=(const classifier_type & classifier_fun) {
        this->m_classifier_fun = classifier_fun;
        return *this;
    }

    operator bool() const {
        return m_classifier_fun != nullptr;
    }

    // Classifies the given input using the classfier function.
    void classify(std::tuple<Args...>& data) {
        std::string _class = utils::apply_func<sizeof...(Args)>::apply(
                m_classifier_fun, data);
        ++m_classes[_class];
        ++m_total;
    }

    // Calculates the frequencies of the classes collected so far
    // and prints the results to out.
    void print_results(std::ostream& out = std::cout) {

        typedef std::pair<size_t, std::string> pair_type;
        std::vector<pair_type> results;

        for (auto &it:m_classes) {
            results.push_back(make_pair(ceil(100 * float(it.second) / m_total), it.first));
        }

        std::sort(results.begin(), results.end(),
                [](const pair_type &p1, const pair_type & p2)
                    //sort in reverse order
                    { return p1.first > p2.first; });

        for (auto &it:results) {
            out<<std::setw(4)<<it.first<<"% "<<it.second<<"."<<std::endl;
        }
    }
};

} //namespace qcppc

#endif // QCPPC_CLASSIFY_H_
