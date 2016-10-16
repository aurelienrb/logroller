#include <boost/log/trivial.hpp>

int main() {
    for (int i = 0; i < 1000 * 1000; ++i) {
        BOOST_LOG_TRIVIAL(trace) << "A trace severity message that is longer than the previous one";
    }
}