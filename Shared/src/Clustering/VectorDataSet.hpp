#ifndef __METRICSPACE_GENERIC_VECTORDATASET_HPP__
#define __METRICSPACE_GENERIC_VECTORDATASET_HPP__

#include "DataSet.hpp"
#include "Containers/Matrix.hpp"
#include "Containers/Array.hpp"
#include "Containers/BitArray.hpp"

namespace Clustering
{
    template<typename VectorType>
    class VectorDataSet : public DataSet<VectorDataSet<VectorType>>
    {
    private:
        Array<VectorType> mVectors;
        unsigned mDim;

    public:
        VectorDataSet (unsigned numVecs, unsigned dim) : mVectors(numVecs, VectorType()) {}
        VectorDataSet (const Array<VectorType>& m, unsigned dim) : mVectors(m), mDim(dim) {}
        VectorDataSet (Array<VectorType>&& m, unsigned dim) : mVectors(std::move(m)), mDim(dim) {}

        VectorDataSet (VectorDataSet&& other) : VectorDataSet(std::move(other.mVectors), other.mDim) {}
        VectorDataSet (const VectorDataSet& other) : VectorDataSet(other.mVectors, other.mDim) {}

        VectorDataSet& operator= (VectorDataSet other)
        {
            std::swap(mVectors, other.mVectors);

            return *this;
        }

        auto& operator[] (unsigned int i)
        {
            return mVectors[i];
        }

        auto& operator[] (unsigned int i) const
        {
            return mVectors[i];
        }

        unsigned int getPointNum () const
        {
            return mVectors.getLength();
        }

        unsigned int getVectorDim() const
        {
            return mDim;
        }
    };

    template<typename T>
    class VectorDataSet<Block<T>> : public DataSet<VectorDataSet<Block<T>>>
    {
    private:
        Matrix<T> mVectors;

    public:
        VectorDataSet (unsigned numVecs, unsigned dim) : mVectors(numVecs, dim) {}
        VectorDataSet (const Matrix<T>&& m) : mVectors(m) {}
        VectorDataSet (Matrix<T>&& m) : mVectors(std::move(m)) {}

        VectorDataSet (VectorDataSet&& other) : VectorDataSet(std::move(other.mVectors)) {}
        VectorDataSet (const VectorDataSet& other) : VectorDataSet(other.mVectors) {}

        VectorDataSet& operator= (VectorDataSet other)
        {
            std::swap(mVectors, other.mVectors);

            return *this;
        }

        auto operator[] (unsigned int i)
        {
            return mVectors.row(i);
        }

        auto operator[] (unsigned int i) const
        {
            return mVectors.row(i);
        }

        unsigned int getPointNum() const
        {
            return mVectors.getColSize();
        }

        unsigned int getVectorDim() const
        {
            return mVectors.getRowSize();
        }
    };
}

#endif /* end of include guard: __METRICSPACE_GENERIC_VECTORDATASET_HPP__ */
