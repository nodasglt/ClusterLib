#ifndef __CLUSTERING_DISTANCEMATRIX_HPP__
#define __CLUSTERING_DISTANCEMATRIX_HPP__

#include "Containers/Array.hpp"
#include "Containers/Matrix.hpp"

#include "DistanceFunction.hpp"

#include <type_traits>

namespace Clustering
{
    namespace Generic
    {
        class DistanceSubMatrix;

        class DistanceMatrix
        {
        public:
            virtual unsigned getPointNum () const = 0;

            virtual double operator() (unsigned x, unsigned y) const = 0;

            virtual DistanceSubMatrix getSubMatrix (Array<unsigned> indices) const;

            virtual ~DistanceMatrix () {};
        };

        class DistanceSubMatrix final : public DistanceMatrix
        {
        public:
            DistanceSubMatrix (const DistanceMatrix& distanceMatrix, Array<unsigned> indices) : mOriginMatrix{&distanceMatrix}, mIndices{std::move(indices)} {}
            DistanceSubMatrix (const DistanceSubMatrix& other) : DistanceSubMatrix(*other.mOriginMatrix, other.mIndices) {};
            DistanceSubMatrix (DistanceSubMatrix&& other) : DistanceSubMatrix(*other.mOriginMatrix, std::move(other.mIndices)) {};

            unsigned getPointNum () const final;

            double operator() (unsigned x, unsigned y) const final;

            DistanceSubMatrix getSubMatrix (Array<unsigned> indices) const final;

        private:
            const DistanceMatrix* mOriginMatrix;
            Array<unsigned> mIndices;
        };
    }

    template<typename Derived>
    class DataSet;

    template<typename DataSetType>
    using DataType = decltype(std::declval<DataSet<DataSetType>>()[unsigned{}]);

    template<typename DistanceFunctionType, typename DataSetType>
    class DistanceMatrix : public Generic::DistanceMatrix
    {
    public:
        DistanceMatrix (const DataSetType& dataSet) : mDataSet(&dataSet), mDistanceFunction() {}

        DistanceMatrix (const DistanceMatrix&) = delete;
        DistanceMatrix (DistanceMatrix&&) = delete;

        DistanceMatrix& operator= (DistanceMatrix) = delete;

        unsigned getPointNum () const override
        {
            return mDataSet->getLength();
        }

        double operator() (unsigned x, unsigned y) const override
        {
            return mDistanceFunction((*mDataSet)[x], (*mDataSet)[y]);
        }

        virtual ~DistanceMatrix() {}

    private:
        const DataSetType* mDataSet;
        const DistanceFunctionType mDistanceFunction;
    };

    template<typename DistanceFunctionType, typename DataSetType>
    class CachedDistMatrix : public DistanceMatrix<DistanceFunctionType, DataSetType>
    {
    public:
        CachedDistMatrix(const DataSetType& dataSet, unsigned N) : DistanceMatrix<DistanceFunctionType, DataSetType>(dataSet), mMatrix(N, N)
        {
            for (unsigned i = 0; i < N; ++i)
            {
                for (unsigned j = 0; j < N; ++j)
                {
                    if (i == j) continue;

                    mMatrix(i, j) = -1.0f;
                }
            }
        }

        virtual double operator() (unsigned x, unsigned y) const final
        {
            if (mMatrix(x, y) < -0.5f)
            {
                mMatrix(x, y) = mMatrix(y, x) = DistanceMatrix<DistanceFunctionType, DataSetType>::operator()(x, y);
            }

            return mMatrix(x, y);
        }

    private:
        const Generic::DistanceMatrix* mOrigin;
        mutable Matrix<double> mMatrix;
    };
}

#endif /* end of include guard: __CLUSTERING_DISTANCEMATRIX_HPP__ */
