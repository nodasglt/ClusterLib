#include "CLARA.hpp"

#include "Methods.hpp"

#include "Util/VectorMath.hpp"

#include <limits>

namespace Clustering
{
    CLARA::CLARA (unsigned s) : S(s) {}

    auto CLARA::operator () (const Generic::DistanceMatrix& distMatrix, unsigned numClusters) const -> State
    {
        MinCost<Init::Concentrate, Assign::PAM, Update::PAMSwap> clusteringFunc;

        unsigned subsetSize = 40 + 2 * numClusters;

        double cost = std::numeric_limits<double>::infinity();

        State state = { &distMatrix, {}, {} };

        for(unsigned int i = 0; i < S; ++i)
        {
            Array<unsigned> dataSample = [&]()
            {
                Util::Random random;

                Array<unsigned> indices;

                indices.reserve(subsetSize);

                while(indices.getLength() < subsetSize)
                {
                    int index;

                    do
                    {
                        index = random.nextInt(distMatrix.getPointNum());
                    }
                    while(indices.exists(index));

                    indices.emplaceBack(index);
                }

                return indices;
            }();

            auto result = clusteringFunc(distMatrix.getSubMatrix(dataSample), numClusters);

            for (auto& x : result.medoidSet)
            {
                x = dataSample[x];
            }

            State currentState = { &distMatrix, std::move(result.medoidSet), {} };

            currentState.clusters = Assign::PAM()(currentState);

            double currentCost = Util::sum(partitioningCost(currentState));

            if (currentCost < cost)
            {
                cost = currentCost;
                state = currentState;
            }
        }

        return state;
    }
}
