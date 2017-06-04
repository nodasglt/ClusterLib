#ifndef __CLUSTERING_BRUTEFORCENN_HPP__
#define __CLUSTERING_BRUTEFORCENN_HPP__

#include "../DistanceMatrix.hpp"

#include "Containers/Maybe.hpp"

namespace Clustering
{
    namespace Nearest
    {
        class BruteForce
        {
        public:
            struct QueryResult
            {
                unsigned index;
                double dist;
            };

        public:
            BruteForce (const Generic::DistanceMatrix&, Maybe<Array<unsigned>> selection = {});

            auto operator() (unsigned p) const -> QueryResult;

        private:
            const Generic::DistanceMatrix* mDistanceMatrix;
            Maybe<Array<unsigned>> mSelection;
            auto (BruteForce::*mMethod)(unsigned p) const -> QueryResult;

            auto partial (unsigned p) const -> QueryResult;
            auto complete (unsigned p) const -> QueryResult;
        };
    }
}

#endif /* end of include guard: __CLUSTERING_BRUTEFORCENN_HPP__ */
