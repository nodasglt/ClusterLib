#ifndef __CLUSTERING_METHODS_HPP__
#define __CLUSTERING_METHODS_HPP__

#include "ClusteringFunction.hpp"

namespace Clustering
{
    namespace Init
    {
        struct Concentrate : Stage<StageLabel::Init>
        {
            auto operator() (const Generic::DistanceMatrix& distMatrix, unsigned numClusters) const -> State final;
        };

        struct KMedoids : public Stage<StageLabel::Init>
        {
            auto operator() (const Generic::DistanceMatrix& distMatrix, unsigned numClusters) const -> State final;
        };
    }

    namespace Assign
    {
        struct PAM : Stage<StageLabel::Assign>
        {
            auto operator () (const State& state) const -> Array<Array<unsigned>> final;
        };
    }

    namespace Update
    {
        struct PAMSwap : public Stage<StageLabel::ProbeUpdate>
        {
            struct SamplingState
            {
                unsigned i, j;
                unsigned medoidNum, dataSetSize;

                SamplingState ();

                SamplingState (const State& state);

                bool hasNext() const;

                SamplingState& update();
            };

            auto operator() (const State& state) const -> Maybe<std::pair<Array<unsigned>, std::pair<unsigned, unsigned>>> final;

            bool hasNext () const final;

            void init (const State& state) const final;

        private:
            mutable SamplingState mSample;
        };

        struct Lloyd : Stage<StageLabel::Update>
        {
            auto operator() (const State& state) const -> Array<unsigned> final;
        };

        struct CLARANS : Stage<StageLabel::ProbeUpdate>
        {
            struct SamplingState
            {
                unsigned num;

                bool hasNext() const;

                SamplingState& update();
            };

            CLARANS (unsigned maxSamples);

            auto operator() (const State& state) const -> Maybe<std::pair<Array<unsigned>, std::pair<unsigned, unsigned>>> final;

            bool hasNext () const final;

            void init (const State& state) const final;

        private:
            unsigned mMaxSamples;
            mutable SamplingState mSample;
        };
    }
}

#endif /* end of include guard: __CLUSTERING_METHODS_HPP__ */
