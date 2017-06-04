#include "../DistanceMatrix.hpp"

#include "Containers/Matrix.hpp"

#include "Util/Random.hpp"

#include <cassert>

#include "Containers/BitArray.hpp"

namespace Clustering
{
    class DistanceHash
    {
    public:
        DistanceHash (unsigned hashTablesNum, unsigned functionsPerHashTable, const Generic::DistanceMatrix& distMatrix)
            : mDistMatrix(&distMatrix), mLines(hashTablesNum, functionsPerHashTable)
        {
            assert(functionsPerHashTable <= 64);

            Util::Random uniformRandom;

            auto pointNum = mDistMatrix->getPointNum();

            for (unsigned i = 0; i < hashTablesNum; ++i)
            {
                for (unsigned j = 0; j < functionsPerHashTable; ++j)
                {
                    unsigned x, y;
                    do
                    {
                        x = uniformRandom.nextInt(pointNum - 1) ;
                        y = uniformRandom.nextInt(pointNum - 1) ;
                    }
                    while (x == y);

                    mLines(i, j).x = x;
                    mLines(i, j).y = y;
                    mLines(i, j).length = (*mDistMatrix)(x, y);

                    double sum = 0.0;
                    for(unsigned n = 0; n < pointNum; ++n)
                    {
                        sum += project(mLines(i, j), n);
                    }

                    double midValue = sum / (double)pointNum;

                    mLines(i, j).midValue = midValue;
                }
            }
        }

        uint64_t getKeyAtIndex (unsigned p, unsigned i) const
        {
            BitArray<64> hash;

            for(unsigned j = 0; j < mLines.getRowSize(); j++)
            {
                hash[j] = (project(mLines(i,j), p) >= mLines(i, j).midValue);
            }

            return (uint64_t)hash ;
        }

        Array<Array<unsigned>> getKeys() const
        {
            Array<Array<unsigned>> mKeys(mDistMatrix->getPointNum());

            for (unsigned i = 0; i < mDistMatrix->getPointNum(); ++i)
            {
                mKeys[i].reserve(mLines.getColSize());

                for (unsigned j = 0; j < mLines.getColSize(); ++j)
                {
                    mKeys[i].emplaceBack(getKeyAtIndex(i, j));
                }
            }

            return mKeys;
        }

    private:
        struct line
        {
            unsigned x, y;
            double length;
            double midValue;
        };

        const Generic::DistanceMatrix* mDistMatrix;

        Matrix<line> mLines;

        double project (line l, unsigned indexToProject) const
        {
            auto distX = (*mDistMatrix)(indexToProject, l.x);
            auto distY = (*mDistMatrix)(indexToProject, l.y);

            return (distX * distX + distY * distY + l.length * l.length) / (2.0f * l.length);
        }
    };
}
