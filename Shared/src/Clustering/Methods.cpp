#include "Methods.hpp"

#include <limits>
#include <assert.h>

#include "Containers/Array.hpp"
#include "Nearest/BruteForce.hpp"

namespace Clustering
{
    namespace Init
    {
        auto Concentrate::operator() (const Generic::DistanceMatrix& distMatrix, unsigned numClusters) const -> State
        {
            Array<double> v(distMatrix.getPointNum());
            Array<double> denominators(distMatrix.getPointNum());

            for (unsigned j = 0; j < distMatrix.getPointNum(); ++j)
            {
                for (unsigned t = j + 1; t < distMatrix.getPointNum(); ++t)
                {
                    auto value = distMatrix(j, t);

                    denominators[j] += value;
                    denominators[t] += value;
                }
            }

            for (unsigned i = 0; i < distMatrix.getPointNum(); ++i)
            {
                for (unsigned j = i + 1; j < distMatrix.getPointNum(); ++j)
                {
                    auto value = distMatrix(i, j);

                    v[i] += value / denominators[j];
                    v[j] += value / denominators[i];
                }
            }

            Array<unsigned> medoids;

            medoids.reserve(numClusters);

            for (unsigned i = 0; i < numClusters; ++i)
            {
                medoids.emplaceBack(i);
            }

            auto getMax = [&]() -> unsigned&
            {
                auto max = medoids.begin();

                for (unsigned* i = max + 1; i < medoids.end(); ++i)
                {
                    if (v[*i] > v[*max])
                    {
                        max = i;
                    }
                }

                return *max;
            };

            for (unsigned i = numClusters; i < v.getLength(); ++i)
            {
                auto& max = getMax();

                if (v[i] < v[max])
                {
                    max = i;
                }
            }

            return { &distMatrix, medoids, {} };
        }

        template<typename ArrayType, typename T>
        T binarySearch (const ArrayType& distances, const T& probe)
        {
            unsigned start = 0, end = distances.getLength(), mid = 0;

            while ((end - start) > 2)
            {
                mid = (end + start) / 2;

                if (distances[mid] < probe)
                {
                    start = mid;
                }
                else
                {
                    end = mid;
                }
            }

            return (distances[mid] <= probe) ? mid : end;
        }

        auto KMedoids::operator() (const Generic::DistanceMatrix& distMatrix, unsigned numClusters) const -> State
        {
            Util::Random uniformRandom;

            Array<unsigned> medoids;
            Array<double> distances(distMatrix.getPointNum());

            medoids.reserve(numClusters);

            medoids.emplaceBack(uniformRandom.nextInt(distMatrix.getPointNum()));

            while (medoids.getLength() < numClusters)
            {
                Nearest::BruteForce near(distMatrix, medoids);

                for (unsigned i = 0; i < distances.getLength(); ++i)
                {
                    distances[i] = near(i).dist;

                    distances[i] *= distances[i];

                    distances[i] += (i > 0) ? distances[i - 1] : 0.0f;
                }

                auto probe = uniformRandom.nextDouble(0.0f, distances[distances.getLength() - 1]);

                medoids.emplaceBack(binarySearch(distances, probe));
            }

            return { &distMatrix, medoids, {} };
        }
    }

    namespace Assign
    {
        auto PAM::operator () (const State& state) const -> Array<Array<unsigned>>
        {
            Array<Array<unsigned>> clusters(state.medoidSet.getLength());

    //        std::cout << "CALLED" << std::endl;

            for (unsigned i = 0; i < clusters.getLength(); ++i)
            {
                clusters[i].emplaceBack(state.medoidSet[i]);
            }

            Nearest::BruteForce near(*state.distMatrix, state.medoidSet);

            for (unsigned i = 0; i < state.distMatrix->getPointNum(); ++i)
            {
                if (state.medoidSet.exists(i)) continue;

                clusters[near(i).index].emplaceBack(i);
            }

            return clusters;
        }
    }

    namespace Update
    {
        PAMSwap::SamplingState::SamplingState () : i(0), j(0), medoidNum(0), dataSetSize(0) {}

        PAMSwap::SamplingState::SamplingState (const State& state)
            : i(0), j(0), medoidNum(state.medoidSet.getLength()), dataSetSize(state.distMatrix->getPointNum()) {}

        bool PAMSwap::SamplingState::hasNext() const
        {
            return (j < dataSetSize && i < medoidNum);
        }

        auto PAMSwap::SamplingState::update() -> SamplingState&
        {
            ++j;

            if (j >= dataSetSize)
            {
                ++i;
                j = 0;
            }

            return *this;
        }

        auto PAMSwap::operator() (const State& state) const -> Maybe<std::pair<Array<unsigned>, std::pair<unsigned, unsigned>>>
        {
            if (state.medoidSet.exists(mSample.j))
            {
                mSample.update();
                return {};
            }

            auto newMedoidSet = state.medoidSet;
            newMedoidSet[mSample.i] = mSample.j;

            std::pair<unsigned, unsigned> change = { mSample.i, mSample.j };

            mSample.update();

            return { newMedoidSet, change };
        }

        bool PAMSwap::hasNext () const
        {
            return mSample.hasNext();
        }

        void PAMSwap::init (const State& state) const
        {
            mSample = SamplingState(state);
        }

        auto Lloyd::operator() (const State& state) const -> Array<unsigned>
        {
            assert(state.clusters.getLength() > 0);

            Array<unsigned> medoids;

            medoids.reserve(state.clusters.getLength());

            for (auto& cluster : state.clusters)
            {
                double value = std::numeric_limits<double>::infinity();
                unsigned index = 0;

                for (auto i : cluster)
                {
                    double cur = 0.0f;

                    for (auto j : cluster)
                    {
                        cur += state.dist(i, j);
                    }

                    if (cur < value)
                    {
                        index = i;
                        value = cur;
                    }
                }

                medoids.emplaceBack(index);
            }

            return medoids;
        }

        bool CLARANS::SamplingState::hasNext() const
        {
            return (num > 0);
        }

        auto CLARANS::SamplingState::update() -> SamplingState&
        {
            --num;
            return *this;
        }

        CLARANS::CLARANS (unsigned maxSamples) : mMaxSamples(maxSamples) {}

        auto CLARANS::operator() (const State& state) const -> Maybe<std::pair<Array<unsigned>, std::pair<unsigned, unsigned>>>
        {
            Util::Random uniformRandom;

            unsigned k = state.medoidSet.getLength();
            unsigned n = state.distMatrix->getPointNum();

            unsigned medoidIndex;
            unsigned pointIndex;

            unsigned x = uniformRandom.nextInt( k * n - 1 );

            medoidIndex = x % k;
            pointIndex = x / k;

            if (state.medoidSet.exists(pointIndex))
            {
                return {};
            }

            auto newMedoidSet = state.medoidSet;
            newMedoidSet[medoidIndex] = pointIndex;

            mSample.update();

            return {{ newMedoidSet, { medoidIndex, pointIndex } }};
        }

        bool CLARANS::hasNext () const
        {
            return mSample.hasNext();
        }

        void CLARANS::init (const State& state) const
        {
            mSample.num = mMaxSamples;
        }
    }
}
