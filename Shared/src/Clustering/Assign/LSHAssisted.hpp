#ifndef __CLUSTERING_ASSIGN_LSHASSISTED__
#define __CLUSTERING_ASSIGN_LSHASSISTED__

#include "../DataSet.hpp"

#include "Containers/Array.hpp"
#include "../Nearest/BasicLSH.hpp"
#include "../Nearest/BruteForce.hpp"

#include "../ClusteringFunction.hpp"

namespace Clustering
{
    template<typename Metric, typename DataSetType>
    struct LSHAssisted : Stage<StageLabel::Assign, Metric, DataSetType>
    {
        template<typename ...Args>
        LSHAssisted(const DataSet<DataSetType>& dataSet, Args && ...args) : mHashFunc(std::forward<Args>(args)...), mNear(mHashFunc, dataSet) {}

        auto operator () (const State<DataSetType>& state) const -> Array<DataSubSet<DataSetType>> override
        {
            Array<DataSubSet<DataSetType>> clusters(state.medoidSet.getPointNum(), state.dataSet->getSubSet({}));

            Array<bool> assigned(state.dataSet->getPointNum(), false);

            double range = [&]() -> double
            {
                double min = std::numeric_limits<double>::infinity();

                typename Metric::DistanceFunction dist;

                for (unsigned i = 0; i < state.medoidSet.getPointNum(); ++i)
                {
                    for (unsigned j = 0; j < state.medoidSet.getPointNum(); ++j)
                    {
                        if (i == j) continue;

                        auto value = dist(state.medoidSet[i], state.medoidSet[j]);

                        if (value < min)
                        {
                            min = value;
                        }
                    }
                }

                return min;
            }();

            bool found = true;

            while(found)
            {
                found = false;

                for (unsigned m = 0; m < state.medoidSet.getPointNum(); ++m)
                {
                    mNear.forEachPointInRange(range, state.medoidSet.translate(m), &assigned, [&](auto i, auto)
                    {
                        assigned[i] = true;
                        found = true;
                        clusters[m].append(i);
                    });
                }

                range *= 2;
            }

            auto nearBruteForce = BruteForce<Metric>(state.medoidSet);

            for (unsigned i = 0; i < state.dataSet->getPointNum(); ++i)
            {
                if (assigned[i]) continue;

                clusters[nearBruteForce((*state.dataSet)[i]).index].append(i);
            }

            return clusters;
        }

    private:
        const typename Metric::HashFunction mHashFunc;
        const BasicLSHIndex<Metric, DataSetType> mNear;
    };
}

#endif /* end of include guard: __CLUSTERING_ASSIGN_LSHASSISTED__ */
