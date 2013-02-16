//Do not add this to the manifest
//this is purely for your benifit, to get something compiling and going.

//the robot state is passed through string message on topic "FState"
/*
Accepted States are:
SafetyDance
ScanWander
PTDoor
Avoid
*/

#include "ros/ros.h"
#include <geometry_msgs/Twist.h>
#include <nav_msgs/Odometry.h>
#include <std_msgs/String.h>
#include <string>

string currState;

int main(int argc, char **argv)
{
  ros::init(argc, argv, "<name of your node here>");
  ros::NodeHandle n;
  ros::NodeHandle s;
  ros::NodeHandle ls;
  ros::Publisher cmd_vel_pub = n.advertise<geometry_msgs::Twist>("cmd_vel", 1000);

  //the state messenger doesn't need the same resolution as the cmd::velocity channel.  
//Thus, we only publish 5 times a second.
  ros::Publisher cmd_vel_pub = s.advertise<geometry_msgs::Twist>("FState", 5);

 n,ros::Duration(30));
    geometry_msgs::Twist msg;


   //we listen a little faster than we publish, since we don't know when it will change
   ros::Subscriber sub = ls.subscribe("chatter", 10, checkStateChange);
  
   ros::topic::waitForMessage<nav_msgs::Odometry>(std::string("odom"),
   
//begin your methodology

//end your methadology


  return 0;
}


//grab the current State
void checkStateChange(const std_msgs::String::ConstPtr& msg)
{
  ROS_INFO("Current State: [%s]", msg->data.c_str());
  currState = *msg;
}
