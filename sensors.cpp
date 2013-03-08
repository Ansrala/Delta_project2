//sensor node
//interprets data from sensors
//posts them to "worldinfo" topic

#define ANGLE_RES 10
#define PI 3.14592

#include "ros/ros.h"
#include <geometry_msgs/Twist.h>
#include <nav_msgs/Odometry.h>
#include <string>
#include <std_msgs/String.h>
#include <sensor_msgs/LaserScan.h>
#include <stdlib.h>
#include <cmath>

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


//setup comparison lines, save this poor gumstick some maths
float constAngles[ANGLE_RES];
float lineSlope[ANGLE_RES];
float cosAngles[ANGLE_RES];
float sinAngles[ANGLE_RES];
float slope[ANGLE_RES];
float diffm1m2[ANGLE_RES];
for(int i = 0; i < ANGLE_RES; i++)
{
	constAngles[i] = PI / ANGLE * i +.001;
	cosAngles[i] = cos(PI / ANGLE * i);
	sinAngles[i] = sin(PI / ANGLE * i);
	slope[i] = tan(constAngles[i]);
	//antiSlope is tan(constAngles[i] + PI/2), soooooo
	diffm1m2 = abs(tan(constAngles[i])) - tan(constAngles[i] + PI/2);
}



  
   ros::Subscriber info_get = sense.subscribe("scan", 500, loadLaser);
   ros::Publisher info_pub = talk.advertise<sensor_msgs::LaserScan>("worldinfo", 500);
   
//begin your methodology

//ok.
//so.
//A Hough transform transforms a set of points into a set of lines.
//as seen here:
//http://en.wikipedia.org/wiki/Hough_transform

//basically, for each point, You draw a bunch of standard lines over it, as defined by you
//for here, it is defined as ANGLE_RES, where the robot will check each point ANGLE_RES times, against each point 
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


int* linearcoordX = new int[ranges.size()];
int* linearcoordY = new int[ranges.size()];

//convert to linear coords here

//end conversion



//make distance matrix
int* distMatrix[ANGLE_RES] = new int[ranges.size()];
int Xcross;
int Ycross;
int c1;

for(int i = 0; i < ranges.size(); i++)
{
	for(int j = 0; j < ANGLE_RES; j++)
	{
		//find y-intercept
		//c1 = y1 - mx1
		c1 = linearcoordY[i] - linearcoordX[i] * slope[j];

		//find the common X coord
		Xcross = c1 / diffm1m2[j];
		Ycross = slope[j] * Xcross + c1;

		distMatris[j][i] = sqrt(Xcross * Xcross + Ycross * Ycross);
	}
}

//compare each column, note points that have similar distances





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
