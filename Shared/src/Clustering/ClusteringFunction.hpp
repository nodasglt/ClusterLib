#ifndef __CLUSTERING_CLUSTERINGFUNCTION__
#define __CLUSTERING_CLUSTERINGFUNCTION__

#include <type_traits>

#include "DistanceMatrix.hpp"

#include "Util/Random.hpp"
#include "Containers/Matrix.hpp"
#include "../Containers/Maybe.hpp"

#include "Nearest/BruteForce.hpp"

namespace Clustering
{
    struct State
    {
        const Generic::DistanceMatrix*  distMatrix;
        Array<unsigned>                 medoidSet;
        Array<Array<unsigned>>          clusters;

        double dist (unsigned x, unsigned y) const;
    };

    enum class StageLabel { Init, Assign, Update, ProbeUpdate };

    template<StageLabel>
    struct Stage {};

    template<>
    struct Stage<StageLabel::Init>
    {
        virtual auto operator() (const Generic::DistanceMatrix&, unsigned numClusters) const -> State = 0;
    };

    template<>
    struct Stage<StageLabel::Assign>
    {
        virtual auto operator() (const State&) const -> Array<Array<unsigned>> = 0;
    };

    template<>
    struct Stage<StageLabel::Update>
    {
        virtual auto operator() (const State&) const -> Array<unsigned> = 0;
    };

    template<>
    struct Stage<StageLabel::ProbeUpdate>
    {
        virtual void init (const State&) const = 0;
        virtual bool hasNext () const = 0;
        virtual auto operator() (const State& state) const -> Maybe<std::pair<Array<unsigned>, std::pair<unsigned, unsigned>>> = 0;
    };

    struct ClusteringFunction
    {
        virtual auto operator() (const Generic::DistanceMatrix& distMatrix, unsigned numClusters) const -> State = 0;
    };

    class MatrixAdapter : public Generic::DistanceMatrix
    {
    public:
        MatrixAdapter(const Matrix<double>& matrix);

        unsigned getPointNum () const final;

        virtual double operator() (unsigned x, unsigned y) const final;

    private:
        const Matrix<double>* mMatrix;
    };

    template<typename InitMethod, typename AssignMethod, typename UpdateMethod>
    struct FixedPoint : public ClusteringFunction
    {
        auto operator() (const Generic::DistanceMatrix& distMatrix, unsigned numClusters) const -> State override
        {
            auto state = mInit(distMatrix, numClusters);

    //        std::cout << "Initial: " << state.medoidSet << std::endl;

            while(true)
            {
                state.clusters = mAssign(state);

        //        std::cout << "Clusters: " << std::endl;

        //        for (auto& x : state.clusters)
        //        {
        //            std::cout << x << std::endl;
        //        }

                auto newMedoidSet = mUpdate(state);

            //    std::cout << "newMedoidSet: " << newMedoidSet << std::endl;

                if (state.medoidSet == newMedoidSet) break;

                state.medoidSet = std::move(newMedoidSet);
            }

        //    std::cout << "Final: " << state.medoidSet << std::endl;

            return state;
        }

        InitMethod mInit;
        AssignMethod mAssign;
        UpdateMethod mUpdate;
    };

    Array<double> partitioningCost(const State& state);

    template<typename InitMethod, typename AssignMethod, typename ProbeMethod>
    struct MinCost : public ClusteringFunction
    {
        struct Config
        {
            State state;
            Array<double> cost;
        };

        template<typename ...Args>
        MinCost (Args && ...args) : mInit(), mAssign(), mUpdate(std::forward<Args>(args)...) {}

        double Assign(const Config& old, Config& cur, std::pair<unsigned, unsigned> change) const
        {
            double delta = 0.0f;

            Nearest::BruteForce bf(*cur.state.distMatrix, cur.state.medoidSet);

            cur.state.clusters = Array<Array<unsigned>>(old.state.clusters.getLength());

            /**/cur.state.clusters[change.first].emplaceBack(change.second);

            for (unsigned i = 0; i < old.state.clusters.getLength(); ++i)
            {
                if (i != change.first)
                {
                    for (auto j : old.state.clusters[i])
                    {
                        cur.cost[j] = old.state.dist(j, change.second);

                        auto thisDelta = cur.cost[j] - old.cost[j];

                        /**/
                        if (j == change.second)
                        {
                            delta += thisDelta;
                            continue;
                        }

                        if (thisDelta < 0.0f)
                        {
                            cur.state.clusters[change.first].emplaceBack(j);

                            delta += thisDelta;
                        }
                        else
                        {
                            cur.state.clusters[i].emplaceBack(j);

                            cur.cost[j] = old.cost[j];
                        }
                    }
                }
                else
                {
                    for (auto j : old.state.clusters[i])
                    {
                        auto result = bf(j);

                        cur.state.clusters[result.index].emplaceBack(j);

                        cur.cost[j] = result.dist;

                        delta += cur.cost[j] - old.cost[j];
                    }
                }
            }

            return delta;
        }

        auto operator() (const Generic::DistanceMatrix& distMatrix, unsigned numClusters) const -> State override
        {
            Config config = { mInit(distMatrix, numClusters), {} };

            config.state.clusters = mAssign(config.state);

            config.cost = partitioningCost(config.state);

            mUpdate.init(config.state);

            while(mUpdate.hasNext())
            {
                auto newMedoidSet = mUpdate(config.state);

                if (!newMedoidSet) continue;

                Config currentConfig = { { config.state.distMatrix, std::move((*newMedoidSet).first), {} }, config.cost };

                auto delta = Assign(config, currentConfig, (*newMedoidSet).second);

                if (delta < 0.0f)
                {
                    config = currentConfig;
                }
            }

            return config.state;
        }

        InitMethod mInit;
        AssignMethod mAssign;
        ProbeMethod mUpdate;
    };
}

#endif /* end of include guard: __CLUSTERING_CLUSTERINGFUNCTION__ */
