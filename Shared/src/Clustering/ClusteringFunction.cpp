#include "ClusteringFunction.hpp"

#ifdef RENDER
#include "Metrics/Test2D.hpp"
#include "Renderer.hpp"
#endif

namespace Clustering
{
    double State::dist (unsigned x, unsigned y) const
    {
        return (*distMatrix)(x, y);
    }

    MatrixAdapter::MatrixAdapter(const Matrix<double>& matrix) : mMatrix(&matrix) {}

    unsigned MatrixAdapter::getPointNum () const
    {
        return mMatrix->getColSize();
    }

    double MatrixAdapter::operator() (unsigned x, unsigned y) const
    {
        return (*mMatrix)(x, y);
    }

    Array<double> partitioningCost(const State& state)
    {
        Array<double> cost(state.distMatrix->getPointNum(), 0.0f);

        for(unsigned i = 0; i < state.clusters.getLength(); ++i)
        {
            for(auto j : state.clusters[i])
            {
                cost[j] = state.dist(j, state.medoidSet[i]);
            }
        }

        return cost;
    }
/*
    //============================================
#ifdef RENDER
    template<typename DataSetType>
    struct FixedPoint<Test2D, DataSetType> : public ClusteringFunction<Test2D, DataSetType>
    {
        FixedPoint (Stage<StageLabel::Init, Test2D, DataSetType>& init,
                    Stage<StageLabel::Assign, Test2D, DataSetType>& assign,
                    Stage<StageLabel::Update, Test2D, DataSetType>& update)
                    : mInit(&init), mAssign(&assign), mUpdate(&update) {}

        State<DataSetType> operator() (const DataSet<DataSetType>& dataSet, unsigned numClusters) const override
        {
            Matrix<GLfloat> colors(numClusters, 3);
            Util::Random random;

            for (unsigned i = 0; i < colors.getColSize(); ++i)
            {
                for (auto& x : colors.row(i))
                {
                    x = random.nextReal<GLfloat>();
                }
            }

            Renderer renderer(1500, 1000);

            renderer.draw(dataSet, {1, 1, 1}, 1, 500);

            auto state = (*mInit)(dataSet, numClusters);

            renderer.draw(state.medoidSet, {1.0f, 0.0f, 0.0f}, 6, 50);

            while(true)
            {
                state.clusters = (*mAssign)(state);

                for (unsigned i = 0; i < state.clusters.getLength(); ++i)
                {
                    renderer.draw(state.clusters[i], colors.row(i), 2, 50);
                }

                auto newMedoidSet = (*mUpdate)(state);

                renderer.clear();

                if (state.medoidSet == newMedoidSet) break;

                renderer.draw(state.medoidSet, {0, 1, 1}, 6, 0);

                state.medoidSet = std::move(newMedoidSet);

                renderer.draw(*state.dataSet, {1, 1, 1}, 1, 0);

                renderer.draw(state.medoidSet, {1.0f, 0.0f, 0.0f}, 6, 0);
            }

            renderer.draw(state.medoidSet, {0, 1, 1}, 10, 0);

            for (unsigned i = 0; i < state.clusters.getLength(); ++i)
            {
                renderer.draw(state.clusters[i], colors.row(i), 2, 0);
            }

            renderer.clear();

            int xc;
            std::cin >> xc;

            return state;
        }

        Stage<StageLabel::Init, Test2D, DataSetType>* mInit;
        Stage<StageLabel::Assign, Test2D, DataSetType>* mAssign;
        Stage<StageLabel::Update, Test2D, DataSetType>* mUpdate;
    };

    template<typename DataSetType>
    struct MinCost<Test2D, DataSetType> : public ClusteringFunction<Test2D, DataSetType>
    {
        MinCost (Stage<StageLabel::Init, Test2D, DataSetType>& init,
                    Stage<StageLabel::Assign, Test2D, DataSetType>& assign,
                    Stage<StageLabel::ProbeUpdate, Test2D, DataSetType>& update)
                    : mInit(&init), mAssign(&assign), mUpdate(&update) {}

        struct Config
        {
            State<DataSetType> state;
            Array<double> cost;
        };

        double Assign(const Config& old, Config& cur, std::pair<unsigned, unsigned> change) const
        {
            typename Test2D::DistanceFunction dist;

            auto& dataSet = *old.state.dataSet;

            double delta = 0.0f;

            for (unsigned i = 0; i < old.state.clusters.getLength(); ++i)
            {
                if (i != change.first)
                {
                    for (unsigned j = 0; j < old.state.clusters[i].getPointNum(); ++j)
                    {
                        auto realIndex = old.state.clusters[i].translate(j);

                        cur.cost[realIndex] = dist(dataSet[realIndex], dataSet[change.second]);

                        auto thisDelta = cur.cost[realIndex] - old.cost[realIndex];

                        if (thisDelta < 0.0f)
                        {
                            cur.state.clusters[change.first].append(realIndex);

                            delta += thisDelta;
                        }
                        else
                        {
                            cur.state.clusters[i].append(realIndex);

                            cur.cost[realIndex] = old.cost[realIndex];
                        }
                    }
                }
                else
                {
                    auto bf = BruteForce<Test2D>(cur.state.medoidSet);

                    for (unsigned j = 0; j < old.state.clusters[i].getPointNum(); ++j)
                    {
                        auto realIndex = old.state.clusters[i].translate(j);

                        auto result = bf(dataSet[realIndex]);

                        cur.state.clusters[result.index].append(realIndex);

                        cur.cost[realIndex] = result.dist;

                        delta += cur.cost[realIndex] - old.cost[realIndex];
                    }
                }
            }

            std::cout << delta << std::endl;

            return delta;
        }

        State<DataSetType> operator() (const DataSet<DataSetType>& dataSet, unsigned numClusters) const override
        {
            Matrix<GLfloat> colors(numClusters, 3);
            Util::Random random;

            for (unsigned i = 0; i < colors.getColSize(); ++i)
            {
                for (auto& x : colors.row(i))
                {
                    x = random.nextReal<GLfloat>();
                }
            }

            Renderer renderer(1500, 1000);

            renderer.draw(dataSet, {1, 1, 1}, 1, 500);

            Config config = { (*mInit)(dataSet, numClusters), {} };

            renderer.draw(config.state.medoidSet, {1.0f, 0.0f, 0.0f}, 6, 5);

            config.state.clusters = (*mAssign)(config.state);

            for (unsigned i = 0; i < config.state.clusters.getLength(); ++i)
            {
                renderer.draw(config.state.clusters[i], colors.row(i), 2, 5);
            }

            config.cost = partitioningCost<Test2D>(config.state);

            mUpdate->init(config.state);

            while(mUpdate->hasNext())
            {
                auto newMedoidSet = (*mUpdate)(config.state);

                if (!newMedoidSet)
                {
                    std::cout << "hit!" << std::endl;
                    continue;
                }

                Config currentConfig = { { config.state.dataSet, std::move((*newMedoidSet).first), decltype(currentConfig.state.clusters)(numClusters, dataSet.getSubSet({})) }, config.cost };

                auto delta = Assign(config, currentConfig, (*newMedoidSet).second);

                if (delta < 0.0f)
                {
                    renderer.clear();

                    for (unsigned i = 0; i < currentConfig.state.clusters.getLength(); ++i)
                    {
                        renderer.draw(currentConfig.state.clusters[i], colors.row(i), 2, 0);
                    }

                    renderer.draw(currentConfig.state.medoidSet, {0, 1, 1}, 6, 100);

                    config = currentConfig;
                }
            }

            renderer.clear();

            for (unsigned i = 0; i < config.state.clusters.getLength(); ++i)
            {
                renderer.draw(config.state.clusters[i], colors.row(i), 2, 0);
            }

            renderer.draw(config.state.medoidSet, {0, 1, 1}, 10, 5);

            renderer.clear();

            std::cout << config.cost << std::endl;

            int xc;
            std::cin >> xc;

            return config.state;
        }

        Stage<StageLabel::Init, Test2D, DataSetType>* mInit;
        Stage<StageLabel::Assign, Test2D, DataSetType>* mAssign;
        Stage<StageLabel::ProbeUpdate, Test2D, DataSetType>* mUpdate;
    };
    #endif
    */
}
