//sensor node
//interprets data from sensors
//posts them to "worldinfo" topic

#include "ros/ros.h"
#include <geometry_msgs/Twist.h>
#include <nav_msgs/Odometry.h>
#include <string>
#include <std_msgs/String.h>
#include <sensor_msgs/LaserScan.h>

using namespace std;



void checkStateChange(const std_msgs::String& msg);

int main(int argc, char **argv)
{
  ros::init(argc, argv, "p2_delta_sensors");
  ros::NodeHandle n;
  ros::NodeHandle s;
  ros::NodeHandle talk;
  ros::NodeHandle sense;
  ros::Publisher cmd_vel_pub = n.advertise<geometry_msgs::Twist>("cmd_vel", 1000);


ros::topic::waitForMessage<nav_msgs::Odometry>(string("odom"), n,ros::Duration(30));
    geometry_msgs::Twist msg;


   //we listen a little faster than we publish, since we don't know when it will change
   ros::Subscriber sub = ls.subscribe("chatter", 10, checkStateChange);
  
   ros::Subscriber info_get = sense.subscribe("scan", 500, loadLaser);
   ros::Publisher info_pub = talk.advertise<sensor_msgs::LaserScan>("worldinfo", 500);
   
//begin your methodology

//end your methadology


  return 0;
}


//grab the current State
void checkStateChange(const std_msgs::String& msg)
{
  ROS_INFO("Current State: [%s]", msg.data.c_str());
  currState = msg.data;
}

//todo:: make info gathering function
void loadLaser(const sensor_msgs::LaserScan& msg)
{

}
