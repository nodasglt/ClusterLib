#ifndef __CLUSTERING_LOCSENHASH_HPP__
#define __CLUSTERING_LOCSENHASH_HPP__

#include <functional>

#include "../DistanceMatrix.hpp"

#include "Containers/Array.hpp"
#include "Containers/HashMap.hpp"

namespace Clustering
{
    class BasicLSH
    {
        struct QueryResult
        {
            bool found;
            unsigned index;
            double dist;
            unsigned sum;
        };

    private:
        const Generic::DistanceMatrix* mDistMatrix;

        //Map hash key to an array of data set indices
        Array<HashMap<uint64_t, Array<unsigned>>> mHashMapArray;

        Array<Array<unsigned>> mDataSetKeyCache;

    public:
        BasicLSH (const Generic::DistanceMatrix& distMatrix, const Array<Array<unsigned>>& keys, unsigned hashMapSize = 100);

        unsigned forEachPointInCluster (unsigned p, const Array<bool>* skip, std::function<void (unsigned)> func) const;

        unsigned forEachPointInRange (double R, unsigned p, const Array<bool>* skip, std::function<void (unsigned, double)> func) const;

        // Reurn the aproximate nearest neighbor
        QueryResult operator() (unsigned p, const Array<bool>* skip = nullptr) const;
    };
}

#endif /* end of include guard: __CLUSTERING_LOCSENHASH_HPP__ */
