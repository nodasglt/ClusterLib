#include "Silhouette.hpp"

namespace Clustering
{
    auto Silhouette(const State& state) -> std::pair<Array<double>, double>
    {
        Array<double> silhouette;
        silhouette.reserve(state.clusters.getLength());

        double globalValue = 0.0f;

        for(auto& cluster : state.clusters)
        {
            //std::cout << cluster.getLength() << std::endl;
            if (cluster.getLength() == 1) return {silhouette, -1.0f};

        	double ClusterSum = 0.0f;

            auto otherMedoids = state.medoidSet;

            otherMedoids.remove(&cluster - state.clusters.begin());

            Nearest::BruteForce near(*state.distMatrix, otherMedoids);

        	for(auto p : cluster)
        	{
        		double Ap = 0.0f;

        		for(auto op : cluster)
        		{
        			Ap += state.dist(p , op);
        		}
        		Ap /= cluster.getLength();

                auto neightbourClusterIndex = near(p).index;

                double Bp = 0.0f;

        		for(auto op : state.clusters[neightbourClusterIndex])
        		{
        			Bp += state.dist(p, op) ;
        		}
        		Bp /= state.clusters[neightbourClusterIndex].getLength();

        		ClusterSum += (Bp - Ap) / ((Ap > Bp) ? Ap : Bp);
        	}

        	auto ClusterSilhouette = ClusterSum / cluster.getLength();
        	silhouette.emplaceBack( ClusterSilhouette );
            globalValue += ClusterSum;
        }

        return { std::move(silhouette), globalValue / state.distMatrix->getPointNum() } ;
    }
}
