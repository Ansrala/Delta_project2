

#include "ros/ros.h"
#include <geometry_msgs/Twist.h>
#include <nav_msgs/Odometry.h>
#include <string>
#include <std_msgs/String.h>

using namespace std;

string currState;

int main(int argc, char **argv)
{ 
  ros::init(argc, argv, "p2_delta_wander");
  ros::NodeHandle n;
  ros::NodeHandle s;
  ros::Publisher cmd_vel_pub = n.advertise<geometry_msgs::Twist>("cmd_vel", 1000);

  ros::topic::waitForMessage<nav_msgs::Odometry>(string("odom"), n, ros::Duration(30));

  geometry_msgs::Twist msg;


  return 0;
}
