#include "Recommend.hpp"

#include "Util/Random.hpp"
#include "Util/VectorMath.hpp"

#include "Clustering/Distance.hpp"
#include "Clustering/Methods.hpp"
#include "Clustering/Silhouette.hpp"
#include "Clustering/CLARA.hpp"

#include "Clustering/Nearest/BasicLSH.hpp"
#include "Clustering/Nearest/DistanceHash.hpp"

using namespace Clustering;

using Folds = Array<std::pair<RatingsDataSet, RatingsDataSet>>;

Matrix<double> detail::vectorize (const RatingsDataSet& ratings)
{
    unsigned max = 0;

    for (auto& pairs : ratings)
    {
        for (auto& p : pairs)
        {
            max = (p.first > max) ? p.first : max;
        }
    }

    Matrix<double> vecs(ratings.getLength(), max);

    for (unsigned i = 0; i < ratings.getLength(); ++i)
    {
        for (auto& p : ratings[i])
        {
            vecs(i, p.first - 1) = p.second;
        }
    }

    return vecs;
}

Array<Array<unsigned>> detail::getClusters (const Generic::DistanceMatrix& distMatrix)
{
    const ClusteringFunction& method = FixedPoint<Init::Concentrate, Assign::PAM, Update::Lloyd>();

    double q = -2.0f;

    State state;

    for (unsigned i = 20, step = 128, skip = 0;; i += step)
    {
        if (skip == i)
        {
            i -= step;
            step /= 2;
            if (!step) break;
            else continue;
        }

        std::cerr << '\r' << "Optimizing k: " << std::setw(4) << i << " : " << q << std::flush;
        //std::this_thread::sleep_for(std::chrono::milliseconds(500));

        auto currentState = method(distMatrix, i);
        auto quality = Silhouette(currentState);

        if (quality.second > q) /*eq ??*/
        {
            q = quality.second;
            state = currentState;
        }
        else
        {
            skip = i;
            i -= step;
            step /= 2;
            if (!step) break;
        }
    }

    std::cerr << "\rAproximate best k: " << state.clusters.getLength() << " with sil: " << q << std::endl;

    //std::cout << state.clusters[5].getLength() << std::endl;

    return state.clusters;
}

Matrix<double> detail::predict (const Matrix<double>& ratings, const Generic::DistanceMatrix& distMatrix, bool flag, std::function<Array<unsigned>(unsigned)> near)
{
    Matrix<double> predicted(ratings);

    Array<unsigned> weights(5, 0);
    for (unsigned i = 0; i < ratings.getColSize(); ++i)
    {
        for (auto x : ratings.row(i))
        {
            auto index = (unsigned)x;
            if (index)
            {
                ++weights[index - 1];
            }
        }
    }

    auto denom = Util::sum(weights);

    for (unsigned i = 0; i < weights.getLength(); ++i)
    {
        weights[i] *= (i + 1);
    }

    auto median = Util::sum(weights) / denom;

    for (unsigned i = 0; i < ratings.getColSize(); ++i)
    {
        Array<unsigned> cluster = near(i);

        std::cerr << '\r' << "Predicting: "  << std::setw(6) << i << std::flush;

        for (unsigned j = 0; j < ratings.getRowSize(); ++j)
        {
            if (ratings(i, j) == 0.0f)
            {
                double z = 0.0f;
                for (unsigned u : cluster)
                {
                    if (ratings(u, j) > 0.0f)
                    {
                        double sim = 1 / (1 + distMatrix(i, u));

                        z += sim;

                        predicted(i, j) += sim * ratings(u, j);
                    }
                }

                if (z > 0.0f)
                {
                    predicted(i, j) /= z;
                }
                else
                {
                    assert(predicted(i, j) == 0.0f);

                    predicted(i, j) = median;
                }
            }
        }
    }

    std::cerr << std::endl;

    return predicted;
}

Array<unsigned> best (unsigned k, Block<double> ratings, const UserRatings& ignore)
{
    Array<unsigned> items;
    items.reserve(k);

    Array<unsigned> ignoreIndices;
    ignoreIndices.reserve(ignore.getLength());

    for (auto x : ignore)
    {
        ignoreIndices.emplaceBack(x.first - 1);
    }

    unsigned offset = 0;
    for (unsigned i = 0; i < k + offset; ++i)
    {
        if (ignoreIndices.exists(i))
        {
            ++offset;
            continue;
        }
        items.emplaceBack(i);
    }

    for (auto& item : items)
    {
        double value = ratings[item];

        for (unsigned i = k + offset; i < ratings.getLength(); ++i)
        {
            if (ratings[i] > value && !items.exists(i) && !ignoreIndices.exists(i))
            {
                item = i;
                value = ratings[i];
            }
        }
    }

//    std::cout << items << std::endl;

    for (auto& x : items)
    {
        ++x;
    }

    return items;
}

Array<unsigned> shuffleIndices(unsigned N)
{
    Array<unsigned> array;
    array.reserve(N);

    for (unsigned i = 0; i < N; ++i)
    {
        array.emplaceBack(i);
    }

    Util::Random random;

    if (array.getLength() > 1)
    {
        unsigned i;
        for (i = 0; i < array.getLength() - 1; i++)
        {
          unsigned j = i + random.nextDouble(0.0f, array.getLength() - i);
          //std::cout << j << std::endl;
          auto t = array[j];
          array[j] = array[i];
          array[i] = t;
        }
    }

    return array;
}

Folds split (const RatingsDataSet& dataSet, unsigned N)
{
    Folds folds;

    folds.reserve(N);

    for (unsigned i = 0; i < N; ++i)
    {
        folds.emplaceBack(dataSet.getLength(), dataSet);
    }

    auto randomIndex = shuffleIndices(N);

    unsigned currentFold = 0;

    for (unsigned i = 0; i < dataSet.getLength(); ++i)
    {
        for (auto& rating : dataSet[i])
        {
            auto& fold = folds[randomIndex[currentFold++ % N]];

            fold.first[i].emplaceBack(rating);
            fold.second[i].remove(fold.second[i].find(rating));
        }
    }

    return folds;
}

Array<Array<unsigned>> detail::nearestK (unsigned k, const Generic::DistanceMatrix& distMatrix)
{
    DistanceHash dshFunc(3, 13, distMatrix);

    BasicLSH lsh(distMatrix, dshFunc.getKeys());

    Array<Array<unsigned>> clusters(distMatrix.getPointNum());

    for (unsigned i = 0; i < distMatrix.getPointNum(); ++i)
    {
        Array<unsigned> all;
        all.reserve(2 * k);

        lsh.forEachPointInCluster(i, nullptr, [&all, i](auto index)
        {
            if (index != i)
            {
                all.emplaceBack(index);
            }
        });

        if (all.getLength() <= k)
        {
            clusters[i] = all;
        }
        else
        {
            clusters[i].reserve(k);

            for (unsigned j = 0; j < k; ++j)
            {
                clusters[i].emplaceBack(all[j]);
            }

            for (auto& item : clusters[i])
            {
                auto value = distMatrix(i, item);

                for (unsigned j = k; j < all.getLength(); ++j)
                {
                    auto currValue = distMatrix(i, all[j]);
                    if (currValue < value && !clusters[i].exists(all[j]))
                    {
                        item = all[j];
                        value = currValue;
                    }
                }
            }
        }
    }

    return clusters;
}

double KFoldEval (const RatingsDataSet& dataSet, unsigned N, std::function<Matrix<double>(const RatingsDataSet&)> predictMethod)
{
    auto folds = split(dataSet, N);

    auto MAE = 0.0f;

    for (auto& fold : folds)
    {
        auto r = predictMethod(fold.second);

        double sum = 0.0f;
        unsigned size = 0;

        for (unsigned i = 0; i < fold.first.getLength(); ++i)
        {
            for (auto& x : fold.first[i])
            {
                auto predicted = r(i, x.first - 1);

                double diff = x.second - predicted;

                if (diff < 0.0f) diff *= -1;

                sum += diff;

                ++size;
            }
        }

        auto currentMAE = sum / size;

        MAE += currentMAE;
    }

    return MAE / folds.getLength();
}
