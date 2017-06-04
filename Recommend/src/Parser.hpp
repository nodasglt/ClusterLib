#ifndef __PARSER_HPP__
#define __PARSER_HPP__

#include "Containers/Array.hpp"

namespace Parser
{
    Array<Array<std::pair<unsigned, double>>> parseRatings (const std::string& fileName);
}

#endif /* end of include guard: __PARSER_HPP__ */
