#include "Parser.hpp"

#include <string>
#include <fstream>
#include <algorithm>
#include <iterator>

namespace Parser
{
    Array<Array<std::pair<unsigned, double>>> parseRatings (const std::string& fileName)
    {
        std::ifstream file(fileName);

        if (!file.is_open()) throw std::runtime_error("File " + fileName + " cannot be oppened.");

        Array<Array<std::pair<unsigned, double>>> dataSet;

        unsigned i = 0;

        while(true)
        {
            unsigned j;
            file >> j;
            if (i != j)
            {
                dataSet.emplaceBack();
                i = j;
            }

            unsigned item;
            double rank;

            file >> item >> rank;

            if (!file.good()) break;

            dataSet[dataSet.getLength() - 1].emplaceBack(item, rank);
        }

        return dataSet;
    }
}
