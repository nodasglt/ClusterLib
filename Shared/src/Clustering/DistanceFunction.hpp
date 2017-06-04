#ifndef __CLUSTERING_DISTANCEFUNCTION__
#define __CLUSTERING_DISTANCEFUNCTION__

namespace Clustering
{
    template<typename Derived, typename ConstPointRef>
    struct DistanceFunction
    {
        auto operator() (ConstPointRef x, ConstPointRef y) const
        {
            return static_cast<Derived const&>(*this)(x, y);
        }
    };
}

#endif /* end of include guard: __CLUSTERING_DISTANCEFUNCTION__ */
