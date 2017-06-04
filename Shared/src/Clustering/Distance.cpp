#include "Distance.hpp"

#include "Util/Random.hpp"
#include "Util/VectorMath.hpp"
#include "Containers/BitArray.hpp"


namespace Clustering
{
    double Distance::L2::operator() (ArrayView<double> x, ArrayView<double> y) const
    {
        double sum = 0.0f;
        for (unsigned int i = 0; i < x.getLength(); ++i)
        {
            double dif = x[i] - y[i];
            sum += dif * dif;
        }
        return sqrt(sum);
    }

    double Distance::Cos::operator() (ArrayView<double> x, ArrayView<double> y) const
    {
        return 1.0f - Util::cosineSim(x, y);
    }

    double Distance::Hamming::operator() (BitArray<1000> x, BitArray<1000> y) const
    {
        return (x ^ y).count();
    }

#ifdef RENDER
    double Distance::Test2D::operator() (ArrayView<GLfloat> x, ArrayView<GLfloat> y) const
    {
        double sum = 0.0f;
        for (unsigned int i = 0; i < x.getLength(); ++i)
        {
            double dif = x[i] - y[i];
            sum += dif * dif;
        }
        return sqrt(sum);
    }
#endif
}
