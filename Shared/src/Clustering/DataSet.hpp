#ifndef __METRICSPACE_GENERIC_DATASET_HPP__
#define __METRICSPACE_GENERIC_DATASET_HPP__

#include <type_traits>
#include <memory>

#include "Containers/Array.hpp"
#include "DistanceMatrix.hpp"

namespace Clustering
{
    template<typename SuperSetType>
    class DataSubSet;

    template<typename DistanceFunctionType, typename DataSetType>
    class DistanceMatrix;

    template<typename Derived>
    class DataSet
    {
    public:
        class Iterator
        {
        public:
            Iterator (const DataSet& dataSet, unsigned i = 0) : mSet(dataSet), mCurIndex(i) {}

            void operator++ ()
            {
                ++mCurIndex;
            }

            auto operator* () const
            {
                return mSet[mCurIndex];
            }

            bool operator== (const Iterator& other) const
            {
                return (mCurIndex == other.mCurIndex);
            }

            bool operator!= (const Iterator& other) const
            {
                return !(*this == other);
            }

            int operator- (const Iterator& other) const
            {
                return (int)mCurIndex - (int)other.mCurIndex;
            }

        private:
            const DataSet& mSet;
            unsigned mCurIndex;
        };

        Iterator begin () const
        {
            return { *this };
        }

        Iterator end () const
        {
            return { *this, getPointNum() };
        }

        auto operator[] (unsigned int i)
        {
            return static_cast<Derived&>(*this)[i];
        }

        auto operator[] (unsigned int i) const
        {
            return static_cast<Derived const&>(*this)[i];
        }

        auto getPointNum () const
        {
            return static_cast<Derived const&>(*this).getPointNum();
        }

        DataSubSet<Derived> getSubSet (Array<unsigned> map) const
        {
            return DataSubSet<Derived>(static_cast<Derived const&>(*this), map);
        }

        Derived& getDerived ()
        {
            return static_cast<Derived&>(*this);
        }

        const Derived& getDerived () const
        {
            return static_cast<Derived const&>(*this);
        }

        template<typename DistanceFunctionType>
        const DistanceMatrix<DistanceFunctionType, Derived>& getDistanceMatrix () const
        {
            static auto x = std::make_unique<DistanceMatrix<DistanceFunctionType, Derived>>(static_cast<Derived const&>(*this));
            return *x.get();
        }
    };
}

#endif /* end of include guard: __METRICSPACE_GENERIC_DATASET_HPP__ */
