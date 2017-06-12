/*
 * unit_test.cpp
 *
 *  Created on: Jun 8, 2017
 *      Author: pauvsi
 */

#include <ros/ros.h>

#include <image_transport/image_transport.h>
#include "std_msgs/String.h"
#include <sstream>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/features2d.hpp>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/xfeatures2d.hpp"
#include "opencv2/video.hpp"

#include <tf2_ros/transform_broadcaster.h>
#include <tf2_ros/message_filter.h>
#include <tf2_ros/transform_listener.h>
#include <tf/tf.h>
#include <tf/transform_listener.h>
#include <tf/tf.h>
#include <tf/tfMessage.h>

#include <eigen3/Eigen/Eigen>

#include "mantis3/Mantis3Params.h"

#include "mantis3/Mantis3Types.h"

#include "mantis3/QuadDetection.h"

#include "mantis3/CoPlanarPoseEstimator.h"


int main(int argc, char **argv)
{
	ros::init(argc, argv, "unit_test");

	ros::NodeHandle nh;

	//load map
	getParameters();

	cv::Mat_<float> D = (cv::Mat_<float>(4, 1) << 0.0029509200248867273, -0.009944040328264236, 0.005587350111454725, -0.00205406011082232);
	cv::Mat K = (cv::Mat_<float>(3, 3) << 323.1511535644531, 0.0, 642.658203125, 0.0, 322.78955078125, 501.5538330078125, 0.0, 0.0, 1.0);

	cv::Mat test = cv::Mat::zeros(1024, 1280, CV_8UC3);

	std::vector<tf::Vector3> object_tf;
	object_tf.push_back(tf::Vector3(0.155, 0.155, 0));
	object_tf.push_back(tf::Vector3(-0.155, 0.155, 0));
	object_tf.push_back(tf::Vector3(-0.155, -0.155, 0));
	object_tf.push_back(tf::Vector3(0.155, -0.155, 0));

	std::vector<cv::Point3d> object_cv;
	object_cv.push_back(cv::Point3d(0.155, 0.155, 0));
	object_cv.push_back(cv::Point3d(-0.155, 0.155, 0));
	object_cv.push_back(cv::Point3d(-0.155, -0.155, 0));
	object_cv.push_back(cv::Point3d(0.155, -0.155, 0));

	cv::Mat_<double> rvec = (cv::Mat_<double>(3, 1) << -2, CV_PI, 0.1);
	cv::Mat_<double> tvec = (cv::Mat_<double>(3, 1) << 0, 0.7, 1);
	cv::Mat_<double> rot;
	cv::Rodrigues(rvec, rot);

	std::vector<cv::Point2d> img_pts;

	Hypothesis actual_hyp;
	actual_hyp.setC2W(actual_hyp.rotAndtvec2tf(rot, tvec));

	ROS_DEBUG_STREAM("actual rot: " << rot);
	ROS_DEBUG_STREAM("actual tvec: " << tvec);

	ROS_DEBUG_STREAM("actual position: " << actual_hyp.getPosition().x() << ", " << actual_hyp.getPosition().y() << ", " << actual_hyp.getPosition().z());

	img_pts.push_back(actual_hyp.distortPixel(actual_hyp.projectPoint(object_tf.at(0)), K, D));
	img_pts.push_back(actual_hyp.distortPixel(actual_hyp.projectPoint(object_tf.at(1)), K, D));
	img_pts.push_back(actual_hyp.distortPixel(actual_hyp.projectPoint(object_tf.at(2)), K, D));
	img_pts.push_back(actual_hyp.distortPixel(actual_hyp.projectPoint(object_tf.at(3)), K, D));

	cv::drawMarker(test, img_pts.at(0), cv::Scalar(255, 255, 255));
	cv::drawMarker(test, img_pts.at(1), cv::Scalar(255, 0, 0));
	cv::drawMarker(test, img_pts.at(2), cv::Scalar(0, 255, 0));
	cv::drawMarker(test, img_pts.at(3), cv::Scalar(0, 0, 255));

	cv::imshow("unit_test", test);
	cv::waitKey(30);

	ros::Duration errorSleep(1);
	errorSleep.sleep();

	ROS_DEBUG("SOLVING FOR POSE");

	std::vector<cv::Point2d> img_pts_normal;

	cv::RNG rng(1);

	double noiseLevel = 0.01;

	img_pts_normal.push_back(actual_hyp.normalizePoint(actual_hyp.projectPoint(object_tf.at(0)) + tf::Vector3(rng.gaussian(noiseLevel), rng.gaussian(noiseLevel), 0)));
	img_pts_normal.push_back(actual_hyp.normalizePoint(actual_hyp.projectPoint(object_tf.at(1)) + tf::Vector3(rng.gaussian(noiseLevel), rng.gaussian(noiseLevel), 0)));
	img_pts_normal.push_back(actual_hyp.normalizePoint(actual_hyp.projectPoint(object_tf.at(2)) + tf::Vector3(rng.gaussian(noiseLevel), rng.gaussian(noiseLevel), 0)));
	img_pts_normal.push_back(actual_hyp.normalizePoint(actual_hyp.projectPoint(object_tf.at(3)) + tf::Vector3(rng.gaussian(noiseLevel), rng.gaussian(noiseLevel), 0)));

	CoPlanarPoseEstimator pe;

	Hypothesis estimate;
	estimate.setC2W(pe.estimatePose(img_pts_normal, object_tf));

	ROS_DEBUG_STREAM("estimate position: " << estimate.getPosition().x() << ", " << estimate.getPosition().y() << ", " << estimate.getPosition().z());




	//loop till end
	while(ros::ok()){
		cv::imshow("unit_test", test);
		cv::waitKey(30);
	}
}

