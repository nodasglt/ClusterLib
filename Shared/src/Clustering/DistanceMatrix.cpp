#include "DistanceMatrix.hpp"

namespace Clustering
{
    namespace Generic
    {
        DistanceSubMatrix DistanceMatrix::getSubMatrix (Array<unsigned> indices) const { return {*this, indices}; }

        unsigned DistanceSubMatrix::getPointNum () const { return mIndices.getLength(); }

        double DistanceSubMatrix::operator() (unsigned x, unsigned y) const
        {
            return (*mOriginMatrix)(mIndices[x], mIndices[y]);
        }

        DistanceSubMatrix DistanceSubMatrix::getSubMatrix (Array<unsigned> indices) const
        {
            for (auto& x : indices)
            {
                x = mIndices[x];
            }

            return {*this, indices};
        }
    }
}
