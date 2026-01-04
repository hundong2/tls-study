#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/version.hpp>

#include "cpprun/hello.hpp"

static std::string join_words(std::string_view input) {
    std::vector<std::string> parts;
    boost::split(parts, std::string(input), boost::is_any_of(" "), boost::token_compress_on);
    return boost::algorithm::join(parts, "|");
}

static int sum(std::span<const int> values) {
    int total = 0;
    for (int v : values) total += v;
    return total;
}

int main() {
    std::cout << cpprun::hello() << "\n";

    std::cout << "boost version: "
              << (BOOST_VERSION / 100000) << '.'
              << (BOOST_VERSION / 100 % 1000) << '.'
              << (BOOST_VERSION % 100) << "\n";

    std::cout << "join demo: " << join_words("a  b   c") << "\n";

    const int xs[] = {1, 2, 3, 4};
    std::cout << "span sum(C++20): " << sum(xs) << "\n";

    return 0;
}
