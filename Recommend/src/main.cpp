#include <iostream>
#include <fstream>
#include <algorithm>
#include <ctime>
#include <random>
#include <iomanip>
#include <chrono>
#include <cassert>

#include "Parser.hpp"

#include "Recommend.hpp"

using namespace Clustering;

template<typename T>
std::ostream& operator<< (std::ostream& os, std::pair<T, T> x)
{
    return os << '{' << x.first << ", " << x.second << '}';
}

struct L2Sparse
{
    double operator() (const UserRatings& x, const UserRatings& y, double max = 30000.0f) const
    {
        double sum = 0.0f;
        bool flag = false;

        for (auto& a : x)
        {
            for (auto& b : y)
            {
                if (a.first == b.first)
                {
                    flag = true;

                    double dif = (double)a.second - (double)b.second;

                    sum += dif * dif;
                }
            }

        }

        return (flag) ? sqrt(sum) : max;
    }
};

struct CosSparse
{
    double operator() (const UserRatings& x, const UserRatings& y) const
    {
        return 1 - L2dist(normalize(x), normalize(y), 0.0f);
    }

private:
    L2Sparse L2dist;

    UserRatings normalize (UserRatings v) const
    {
        assert(v.getLength() > 0);

        double sum = 0.0f;
        for (auto& rating : v)
        {
            sum += rating.second;
        }

        for (auto& rating : v)
        {
            rating.second /= sum;
        }

        return v;
    }
};

struct HammingSparse
{
    double operator() (const UserRatings& x, const UserRatings& y) const
    {
        unsigned sum = 0;

        for (auto& a : x)
        {
            bool diff = true;

            for (auto& b : y)
            {
                if (a.first == b.first)
                {
                    diff = false;
                }
            }

            if (diff)
                sum++;
        }

        return sum;
    }
};

Array<std::string> argsToArray (int argc, char const* argv[]);

bool isNumber(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(),
        s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
}

int main (int argc, char const* argv[])
{
    std::srand(std::time(nullptr));

    Array<std::string> args = argsToArray(argc, argv);

    if (!args.exists("-d"))
    {
        std::cerr << "Data file required. Use -d flag to specify it." << std::endl;
        return 1;
    }

    if (args.exists("--hamming") && args.exists("--cos"))
    {
        std::cerr << "Multiple metrics specified." << std::endl;

        return 3;
    }

    auto dataSet = Parser::parseRatings(args.at(args.find("-d") + 1));

    std::function<Matrix<double>(const RatingsDataSet&)> method;

    if (args.exists("--lsh"))
    {
        unsigned N = 20;

        if (args.exists("-N"))
        {
            auto& num = args.at(args.find("-N") + 1);

            if (isNumber(num))
            {
                N = std::stoi(num);
            }
            else
            {
                std::cerr << "Expecting integer after -N flag" << std::endl;
                return 2;
            }
        }

        using namespace std::placeholders;

        if (args.exists("--hamming"))
        {
            method = std::bind(predictWithLSH<HammingSparse>, _1, N);
        }
        else if (args.exists("--cos"))
        {
            method = std::bind(predictWithLSH<CosSparse>, _1, N);
        }
        else
        {
            method = std::bind(predictWithLSH<L2Sparse>, _1, N);
        }
    }
    else
    {
        if (args.exists("--hamming"))
        {
            method = predictWithClustering<HammingSparse>;
        }
        else if (args.exists("--cos"))
        {
            method = predictWithClustering<CosSparse>;
        }
        else
        {
            method = predictWithClustering<L2Sparse>;
        }
    }

    if (args.exists("--eval"))
    {
        unsigned F = 20;

        if (args.exists("-F"))
        {
            auto& num = args.at(args.find("-F") + 1);

            if (isNumber(num))
            {
                F = std::stoi(num);
            }
            else
            {
                std::cerr << "Expecting integer after -F flag" << std::endl;
                return 2;
            }
        }

        std::cout << "MAE: " << KFoldEval(dataSet, F, method) << std::endl;
    }
    else
    {
        unsigned T = 10;
        if (args.exists("-T"))
        {
            auto& num = args.at(args.find("-T") + 1);

            if (isNumber(num))
            {
                T = std::stoi(num);
            }
            else
            {
                std::cerr << "Expecting integer after -T flag" << std::endl;
                return 2;
            }
        }

        auto predicted = method(dataSet);

        if (args.exists("-o"))
        {
            std::ofstream out(args.at(args.find("-o") + 1));

            for (unsigned i = 0; i < predicted.getColSize(); ++i)
            {
                out << '[' << i + 1 << "] : " << best(T, predicted.row(i), dataSet[i]) << std::endl;
            }
        }
        else
        {
            for (unsigned i = 0; i < predicted.getColSize(); ++i)
            {
                std::cout << '[' << i + 1 << "] : " << best(T, predicted.row(i), dataSet[i]) << std::endl;
            }
        }
    }

    return 0;
}

Array<std::string> argsToArray (int argc, char const* argv[])
{
    Array<std::string> array;
    array.reserve(argc);

    for (auto i = 0; i < argc; ++i)
    {
        array.emplaceBack(argv[i]);
    }

    return array;
}
