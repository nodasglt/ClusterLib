#include "BasicLSH.hpp"

#include <limits>
#include <cmath>
#include <cassert>

namespace Clustering
{
    BasicLSH::BasicLSH (const Generic::DistanceMatrix& distMatrix, const Array<Array<unsigned>>& keys, unsigned hashMapSize)
        : mDistMatrix(&distMatrix), mHashMapArray(), mDataSetKeyCache(keys)
    {
        assert(mDataSetKeyCache.getLength() > 0);

        mHashMapArray.reserve(mDataSetKeyCache[0].getLength());

        for (unsigned i = 0; i < mDataSetKeyCache[0].getLength(); ++i)
        {
            mHashMapArray.emplaceBack(hashMapSize);
        }

        for (unsigned i = 0; i < mDataSetKeyCache.getLength(); ++i)
        {
            for (unsigned j = 0; j < mHashMapArray.getLength(); ++j)
            {
                auto key = mDataSetKeyCache[i][j];

                if (mHashMapArray[j].exists(key))
                {
                    mHashMapArray[j][key].emplaceBack(i);
                }
                else
                {
                    mHashMapArray[j].add(key, Array<unsigned>(1, i));
                }
            }
        }
    }

    unsigned BasicLSH::forEachPointInCluster (unsigned p, const Array<bool>* skip, std::function<void (unsigned)> func) const
    {
        unsigned sum = 0;

        Array<bool> checked;

        if (skip == nullptr)
        {
            checked = Array<bool>(mDistMatrix->getPointNum(), false);
        }
        else
        {
            assert(skip->getLength() == mDistMatrix->getPointNum());

            checked = *skip;
        }

        for (unsigned i = 0; i < mHashMapArray.getLength(); ++i)
        {
            auto key = mDataSetKeyCache[p][i];
            if (!mHashMapArray[i].exists(key)) continue;
            for (auto x : mHashMapArray[i][key])
            {
                if (!checked[x])
                {
                    sum++;

                    func(x);

                    checked[x] = true;
                }
            }
        }

        return sum;
    }

    unsigned BasicLSH::forEachPointInRange (double R, unsigned p, const Array<bool>* skip, std::function<void (unsigned, double)> func) const
    {
        return forEachPointInCluster(p, skip, [=](auto index)
        {
            double dist = (*mDistMatrix)(p, index);

            if (dist < R)
            {
                func(index, dist);
            }
        });
    }

    // Reurn the aproximate nearest neighbor
    auto BasicLSH::operator() (unsigned p, const Array<bool>* skip) const -> QueryResult
    {
        double minDist = std::numeric_limits<double>::infinity();
        unsigned minIndex = 0;

        unsigned sum = forEachPointInCluster(p, skip, [&](auto index)
        {
            double dist = (*mDistMatrix)(p, index);

            if (dist < minDist)
            {
                minDist = dist;
                minIndex = index;
            }
        });

        return {!std::isinf(minDist), minIndex, minDist, sum};
    }
}
