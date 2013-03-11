//sensor node
//interprets data from sensors
//posts them to "worldinfo" topic

#define ANGLE_RES 10
#define SKIP_MAX 2
#define THRESHOLD .2

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
#include <p2_delta/lineList.h>
#include <p2_delta/pointList.h>


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
float cosAngles[ANGLE_RES];
float sinAngles[ANGLE_RES];
float slope[ANGLE_RES];
float diffm1m2[ANGLE_RES];
for(int i = 0; i < ANGLE_RES; i++)
{
	constAngles[i] = PI / ANGLE_RES * i +.001;
	cosAngles[i] = cos(PI / ANGLE_RES * i);
	sinAngles[i] = sin(PI / ANGLE_RES * i);
	slope[i] = tan(constAngles[i]);
	//antiSlope is tan(constAngles[i] + PI/2), soooooo
	diffm1m2[i] = abs(tan(constAngles[i])) - tan(constAngles[i] + PI/2);
}



  
   ros::Subscriber info_get = sense.subscribe("scan", 500, loadLaser);
   ros::Publisher info_pub1 = talk.advertise<p2_delta::pointList>("worldinfoPoints", 500);
   ros::Publisher info_pub2 = talk.advertise<p2_delta::lineList>("worldinfoLines", 500);

   p2_delta::lineList linesOut;
   p2_delta::pointList pointsOut;
   
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

while(ros::ok())
{

int* linearcoordX = new int[now.ranges.size()];
int* linearcoordY = new int[now.ranges.size()];



//convert to linear coords here
for (int i = 0; i < now.ranges.size(); i++)
{
	linearcoordX[i] = now.ranges[i] * cos(now.angle_min + i * now.angle_increment);
	linearcoordY[i] = now.ranges[i] * sin(now.angle_min + i * now.angle_increment);

}

//end conversion


//make distance matrix
int* distMatrix[ANGLE_RES];
for(int i = 0; i < ANGLE_RES; i++)
	distMatrix[i] = new int[now.ranges.size()];
int Xcross;
int Ycross;
int c1;

for(int i = 0; i < now.ranges.size(); i++)
{
	for(int j = 0; j < ANGLE_RES; j++)
	{
		//find y-intercept
		//c1 = y1 - mx1
		c1 = linearcoordY[i] - linearcoordX[i] * slope[j];

		//find the common X coord
		Xcross = c1 / diffm1m2[j];
		Ycross = slope[j] * Xcross + c1;

		distMatrix[j][i] = sqrt(Xcross * Xcross + Ycross * Ycross);
	}
}

//compare each column, note points that have similar distances
float last = -50;
bool onAline =  false;
int skips = 0;
float lastHitX = 0;
float lastHitY = 0;

for(int j = 0; j < now.ranges.size(); j++)
{

	for(int i = 0; i < ANGLE_RES; i++)
	{
		// is like the last point
		if(distMatrix[i][j] > last - THRESHOLD && distMatrix[i][j] < last + THRESHOLD  )
		{
			//new line
			if(!onAline)
			{
				onAline = true;
				skips = 0;
				//create new line	

				linesOut.x1.push_back(linearcoordX[j-1]);
				linesOut.y1.push_back(linearcoordY[j-1]);

				linesOut.x2.push_back(linearcoordX[j]);
				linesOut.y2.push_back(linearcoordY[j]);

				lastHitX = linesOut.x2.back();
				lastHitY = linesOut.y2.back();
			}
			else 
			{
				skips = 0;
				linesOut.x2.back() = linearcoordX[j];
				linesOut.y2.back() = linearcoordY[j];

				lastHitX = linesOut.x2.back();
				lastHitY = linesOut.y2.back();
			
			}
				
			
		}
		//strayed from the line
		else
		{
			skips++;
			//grace period
			if(skips < SKIP_MAX)
			{
				//add anyway
				linesOut.x2.back() = linearcoordX[j];
				linesOut.y2.back() = linearcoordY[j];
			}
			//new line
			else
			{
				onAline = false;
				linesOut.x2.back() = lastHitX;
				linesOut.y2.back() = lastHitY;

				for(int k = 0; k < skips; k++)
				{
					pointsOut.x.push_back(linearcoordX[j-k]);
					pointsOut.y.push_back(linearcoordY[j-k]);
				}
			}
		}

		last = distMatrix[i][j];
	}
	last = -50;
}
info_pub1.publish(pointsOut);
info_pub2.publish(linesOut);


delete [] linearcoordY;
delete [] linearcoordX;
for(int i = 0; i < ANGLE_RES; i++)
	delete [] distMatrix[i];

}



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

	for(int i = 0; i < msg.ranges.size(); i++)
	{
		now.ranges.push_back(msg.ranges[i]);
	}

}
