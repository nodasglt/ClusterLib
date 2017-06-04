#ifndef __CLUSTERING_SILOUETTE__
#define __CLUSTERING_SILOUETTE__

#include "Containers/Array.hpp"

#include "Clustering/ClusteringFunction.hpp"

namespace Clustering
{
    auto Silhouette(const State& state) -> std::pair<Array<double>, double>;
}

#endif /* end of include guard: __CLUSTERING_SILOUETTE__ */
