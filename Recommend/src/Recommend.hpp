#include <functional>

#include "Containers/Matrix.hpp"
#include "Containers/Array.hpp"

#include "Clustering/DistanceMatrix.hpp"

using UserRatings = Array<std::pair<unsigned, double>>;
using RatingsDataSet = Array<UserRatings>;

using DistanceMatrixType = Clustering::Generic::DistanceMatrix;

namespace detail
{
    Array<Array<unsigned>> getClusters (const DistanceMatrixType&);
    Matrix<double> predict (const Matrix<double>& ratings, const DistanceMatrixType&, bool flag, std::function<Array<unsigned>(unsigned)>);
    Array<Array<unsigned>> nearestK (unsigned, const DistanceMatrixType&);
    Matrix<double> vectorize (const RatingsDataSet&);
}

Array<unsigned> best (unsigned k, Block<double> ratings, const UserRatings& ignore);

double KFoldEval (const RatingsDataSet& dataSet, unsigned N, std::function<Matrix<double>(const RatingsDataSet&)> predictMethod);

template<typename DistanceFunctionType>
auto predictWithClustering (const RatingsDataSet& dataSet) -> Matrix<double>
{
    auto r = detail::vectorize(dataSet);

    Clustering::CachedDistMatrix<DistanceFunctionType, RatingsDataSet> distMatrix(dataSet, r.getColSize());

    auto clusters = detail::getClusters(distMatrix);

    r = detail::predict(r, distMatrix, true, [&clusters](unsigned i)
    {
        for (auto& x : clusters)
        {
            if (x.exists(i))
            {
                return x;
            }
        }

        throw;
    });

    return r;
}

template<typename DistanceFunctionType>
auto predictWithLSH (const RatingsDataSet& dataSet, unsigned k) -> Matrix<double>
{
    auto r = detail::vectorize(dataSet);

    Clustering::CachedDistMatrix<DistanceFunctionType, RatingsDataSet> distMatrix(dataSet, r.getColSize());

    auto clusters = detail::nearestK(k, distMatrix);

    r = detail::predict(r, distMatrix, true, [&clusters](unsigned i)
    {
        return clusters[i];
    });

    return r;
}
