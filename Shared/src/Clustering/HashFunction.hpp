#ifndef __METRICSPACE_GENERIC_METRIC_HPP__
#define __METRICSPACE_GENERIC_METRIC_HPP__

#include <cinttypes>

#include "Containers/Array.hpp"

namespace Clustering
{
    template<typename Metric>
    struct HashFunction
    {
        using ConstPointRef = typename Metric::ConstPointRef;
        using Derived = typename Metric::HashFunction;

        struct KeyGenerator
        {
            const HashFunction& mHashData;
            ConstPointRef mPoint;

            constexpr KeyGenerator (const HashFunction& hashData, ConstPointRef dataPoint) : mHashData(hashData), mPoint(dataPoint) {}

            uint64_t operator[] (unsigned int i) const
            {
                return static_cast<Derived const&>(mHashData).getKeyAtIndex(mPoint, i);
            }

            operator Array<unsigned> () const
            {
                Array<unsigned> keys;

                keys.reserve(mHashData.getKeyNum());

                for (unsigned i = 0; i < mHashData.getKeyNum(); ++i)
                {
                    keys.pushBack((*this)[i]);
                }

                return keys;
            }
        };

        explicit HashFunction (unsigned int keyNum) : mKeyNum(keyNum) {}

        unsigned int getKeyNum() const
        {
            return mKeyNum;
        }

        const KeyGenerator operator() (ConstPointRef p) const
        {
            return KeyGenerator(*this, p);
        }

     private:
        const unsigned mKeyNum;
    };
}

#endif /* end of include guard: __METRICSPACE_GENERIC_METRIC_HPP__ */
