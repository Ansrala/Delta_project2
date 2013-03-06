//sensor node
//interprets data from sensors
//posts them to "worldinfo" topic

#define ANGLE_RES 10

#include "ros/ros.h"
#include <geometry_msgs/Twist.h>
#include <nav_msgs/Odometry.h>
#include <string>
#include <std_msgs/String.h>
#include <sensor_msgs/LaserScan.h>

//our custom messages
#include <Delta_project2/lineList.h>
#include <Delta_project2/pointList.h>


using namespace std;

sensor_msgs::LaserScan now;

void loadLaser(const sensor_msgs::LaserScan& msg);

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



  
   ros::Subscriber info_get = sense.subscribe("scan", 500, loadLaser);
   ros::Publisher info_pub = talk.advertise<sensor_msgs::LaserScan>("worldinfo", 500);
   
//begin your methodology

//ok.
//so.
//A Hough transform transforms a set of points into a set of lines.
//as seen here:
//http://en.wikipedia.org/wiki/Hough_transform

//basically, for each point, You draw a bunch of standard lines over it, as defined by you
//for here, it is defined as ANGLE_RES, where the robot will check each point 10 times, against each line 
//Where each line is an angle 180/ANGLE_RES
//for each line, a perpendicular line is drawn from your defined line to the orgin, and the distance measured
//these results are stored in a matrix angleOfTestLine to distance measured

//if your matrix has a set of rows or partial rows who have similar (within a thresh-hold) distances, the referring points lock to your test line,
//and can be considered a line

//other notes:
//perpendicular vectors can be found by switching x and y coords once centered on orgin
//slope = (y2-y1)/(x2-x1)
//normal slope = -(y2-y1)/(x2-x1) or (y2-y1)/-(x2-x1)
//y1 = slope(x1) + c
//c = y1 - slope(x1)


//end your methadology


  return 0;
}




//todo:: make info gathering function
void loadLaser(const sensor_msgs::LaserScan& msg)
{
	//make a deep copy
	//these are in radians
	now.angle_min = msg.angle_min;
	now.angle_max = msg.angle_max;
	now.angle_increment = msg.angle_increment;

	//time info
	now.time_increment = msg.time_increment;
	now.scan_time = msg.scan_time;	


}
