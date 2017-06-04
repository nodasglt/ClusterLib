#include "BruteForce.hpp"

#include <limits>

namespace Clustering
{
    namespace Nearest
    {
        BruteForce::BruteForce (const Generic::DistanceMatrix& distanceMatrix, Maybe<Array<unsigned>> selection) : mDistanceMatrix(&distanceMatrix), mSelection(std::move(selection))
        {
            mMethod = (mSelection) ? &BruteForce::partial : &BruteForce::complete;
        }

        auto BruteForce::operator() (unsigned p) const -> QueryResult
        {
            return (*this.*mMethod)(p);
        }

        auto BruteForce::partial (unsigned p) const -> QueryResult
        {
            double dist = std::numeric_limits<double>::infinity();
            unsigned index = 0;

            for (unsigned i = 0; i < (*mSelection).getLength(); ++i)
            {
                double curDist = (*mDistanceMatrix)(p, (*mSelection)[i]);
                if (curDist < dist)
                {
                    dist = curDist;
                    index = i;
                }
            }

            return { index, dist };
        }

        auto BruteForce::complete (unsigned p) const -> QueryResult
        {
            if (mSelection) return partial(p);

            double dist = std::numeric_limits<double>::infinity();
            unsigned index = 0;

            for (unsigned i = 0; i < mDistanceMatrix->getPointNum(); ++i)
            {
                double curDist = (*mDistanceMatrix)(p, i);
                if (curDist < dist)
                {
                    dist = curDist;
                    index = i;
                }
            }

            return { index, dist };
        }
    }
}
