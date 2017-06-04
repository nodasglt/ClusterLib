#ifndef __CLUSTERING_CLARA__
#define __CLUSTERING_CLARA__

#include "ClusteringFunction.hpp"

namespace Clustering
{
    struct CLARA : public ClusteringFunction
    {
        unsigned S;

        CLARA (unsigned s);

        auto operator () (const Generic::DistanceMatrix& distMatrix, unsigned numClusters) const -> State final;
    };
}

#endif /* end of include guard: __CLUSTERING_CLARA__ */
