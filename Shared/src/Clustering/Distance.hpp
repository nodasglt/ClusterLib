#ifndef __METRICSPACE_EUCLIDEAN_COS_METRIC_HPP__
#define __METRICSPACE_EUCLIDEAN_COS_METRIC_HPP__

#include "DistanceFunction.hpp"

#include "Containers/Array.hpp"
#include "Containers/Matrix.hpp"
#include "Containers/ArrayView.hpp"
#include "Containers/BitArray.hpp"

#ifndef RENDER
#include <GLFW/glfw3.h>
#endif

namespace Clustering
{
    struct Distance
    {
        struct L2 : public DistanceFunction<L2, ArrayView<double>>
        {
            double operator() (ArrayView<double> x, ArrayView<double> y) const;
        };

        struct Cos : public DistanceFunction<Cos, ArrayView<double>>
        {
            double operator() (ArrayView<double> x, ArrayView<double> y) const;
        };

        struct Hamming : public DistanceFunction<Hamming, BitArray<1000>>
        {
            double operator() (BitArray<1000> x, BitArray<1000> y) const;
        };

#ifdef RENDER
        struct Test2D : public DistanceFunction<Test2D, ArrayView<GLfloat>>
        {
            double operator() (ArrayView<GLfloat> x, ArrayView<GLfloat> y) const;
        };
#endif
    };
}

#endif /* end of include guard: __METRICSPACE_EUCLIDEAN_COS_METRIC_HPP__ */
