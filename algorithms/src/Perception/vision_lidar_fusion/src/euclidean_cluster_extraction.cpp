#include <pcl/pcl_base.h>
#include <pcl/point_types.h>
#include <pcl_conversions/pcl_conversions.h>
#include <pcl_ros/point_cloud.h>
#include <pcl/segmentation/extract_clusters.h>
#include <pcl/filters/extract_indices.h>

#include <ros/ros.h>
#include <glog/logging.h>
#include "Timer.h"


using std::string;
using std::vector;
using std::cout;
using std::endl;
typedef pcl::PointXYZI PointT;
typedef pcl::PointCloud<PointT> LidarCloudT;



typedef pcl::PointXYZRGB VisPointT;
typedef pcl::PointCloud<VisPointT> VisCloudT;

typedef pcl::PCLPointCloud2 MessageCloudT;

ros::NodeHandle* pNH = nullptr;
ros::Publisher* pPub = nullptr;


void doEuclideanSegment (const LidarCloudT::Ptr &cloud_in, vector<LidarCloudT::Ptr> &output,
                         int min_cluster_size, int max_cluster_size, double cluster_tolerance)
{
    // Convert data to PointCloud<T>
    //LidarCloudT::Ptr cloud_in (new LidarCloudT);
    //pcl::fromPCLPointCloud2 (*cloud_msg, *cloud_in);

    // Estimate
    //TicToc tt;
    //tt.tic ();

    LOG(INFO)<<"In doEuclideanSegment()."<<endl;
    ScopeTimer seg_timer("[EuclideanClusterExtraction] in doEuclideanSegment()");
    // Creating the KdTree object for the search method of the extraction
    pcl::search::KdTree<PointT>::Ptr tree (new pcl::search::KdTree<PointT>);
    tree->setInputCloud (cloud_in);

    std::vector<pcl::PointIndices> cluster_indices;
    pcl::EuclideanClusterExtraction<PointT> ec;
    ec.setClusterTolerance (cluster_tolerance);
    ec.setMinClusterSize (min_cluster_size);
    ec.setMaxClusterSize (max_cluster_size);
    ec.setSearchMethod (tree);
    ec.setInputCloud (cloud_in);
    ec.extract (cluster_indices);
    seg_timer.watch("[EuclideanClusterExtraction] extract clusters finished.");

    output.reserve (cluster_indices.size ());
    for (std::vector<pcl::PointIndices>::const_iterator it = cluster_indices.begin (); it != cluster_indices.end (); ++it)
    {
//        pcl::ExtractIndices<LidarCloudT> extract;
//        extract.setInputCloud (cloud_in);
//        extract.setIndices (boost::make_shared<const pcl::PointIndices> (*it));

        LidarCloudT::Ptr out (new LidarCloudT);
        for(auto i:it->indices)
        {
            out->points.push_back(cloud_in->points.at(i));
        }
        out->height = 1;
        out->width = out->size();
        output.push_back (out);
        LOG(INFO)<<"[EuclideanClusterExtraction]    cloud size:"<<out->size()<<endl;
    }
    seg_timer.watch("[EuclideanClusterExtraction] generate cluster clouds finished.");
    LOG(INFO)<<"doEuclideanSegment() Finished."<<endl;
}
void visualizeClusters(const vector<LidarCloudT::Ptr>& clusters)
{
    ScopeTimer vis_timer("[EuclideanClusterExtraction] in visualizeClusters()");
    vector<VisCloudT> vclouds;
    vclouds.reserve(clusters.size());
    for(int i = 0;i<clusters.size();i++)
    {
        vclouds.push_back(VisCloudT());
        unsigned char r,g,b;
        LidarCloudT::Ptr pCurrentCloud = clusters.at(i);
        r = i*1000%256;
        g = i*12429%256;
        b = pCurrentCloud->size()*43112%256;
        for(const PointT& pt:pCurrentCloud->points)
        {
            VisPointT vp;
            vp.x = pt.x;
            vp.y = pt.y;
            vp.z = pt.z;
            vp.r = r;
            vp.g = g;
            vp.b = b;
            vclouds.at(i).push_back(vp);
        }
        vclouds.at(i).height = 1;
        vclouds.at(i).width = vclouds.at(i).points.size();

    }
    vis_timer.watch("[EuclideanClusterExtraction] clusters copied.");
    //publish these clouds.
    for(const auto& vcloud:vclouds)
    {
        MessageCloudT pc;
        pcl::toPCLPointCloud2(vcloud,pc);
        pc.header.frame_id="lidar";
        //pc.header.stamp = ros::Time::now();
        pPub->publish(pc);
    }
    vis_timer.watch("[EuclideanClusterExtraction] clusters published.");
}
void callback(const pcl::PCLPointCloud2::ConstPtr &cloud_msg)
{
    vector<LidarCloudT::Ptr> output_clusters;
    const int min_cluster_size = 100;
    const int max_cluster_size = 20000;
    const double cluster_tolerance = 0.1;
    LidarCloudT::Ptr pCurrentCloud(new LidarCloudT);
    pcl::fromPCLPointCloud2(*cloud_msg,*pCurrentCloud);
    doEuclideanSegment(pCurrentCloud,output_clusters,min_cluster_size,max_cluster_size,cluster_tolerance);
    visualizeClusters(output_clusters);
}

int main(int argc,char **argv)
{
    FLAGS_alsologtostderr = 1;
    google::InitGoogleLogging(argv[0]);
    LOG(INFO)<<"Start euclidean_cluster_fusion_node."<<endl;

    ros::init(argc,argv,"euclidean_cluster_fusion_node");
    ros::NodeHandle nh;
    pNH=&nh;
    ros::Publisher pub = nh.advertise<MessageCloudT>("extracted_euclidean_clusters",10);
    pPub = &pub;
    ros::Subscriber sub = nh.subscribe<MessageCloudT>("velodyne_points2",1,callback);

    ros::spin();
    return 0;
}
