//sensor node
//interprets data from sensors
//posts them to "worldinfo" topic

#define ANGLE_RES 10
#define SKIP_MAX 9
#define THRESHOLD .4f

#define PI 3.14592f

#include "ros/ros.h"
#include <geometry_msgs/Twist.h>
#include <nav_msgs/Odometry.h>
#include <string>
#include <std_msgs/String.h>
#include <sensor_msgs/LaserScan.h>
#include <stdlib.h>
#include <cmath>
//#include <iostream>
//#include <vector>

//our custom messages
#include <p2_delta/lineList.h>
#include <p2_delta/pointList.h>


using namespace std;

sensor_msgs::LaserScan now;

void loadLaser(const sensor_msgs::LaserScan& msg);

void checkStateChange(const std_msgs::String& msg);

void cleanLinesOut(p2_delta::lineList* linesOut);

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

float* linearcoordX = new float[now.ranges.size()];
float* linearcoordY = new float[now.ranges.size()];

cleanLinesOut(&linesOut);

//convert to linear coords here
//ROS_INFO("making linear matrix");
for (int i = 0; i < now.ranges.size(); i++)
{
	linearcoordX[i] = now.ranges[i] * cos(now.angle_min + i * now.angle_increment);
	linearcoordY[i] = now.ranges[i] * sin(now.angle_min + i * now.angle_increment);

}

//end conversion


//make distance matrix
float* distMatrix[ANGLE_RES];
for(int i = 0; i < ANGLE_RES; i++)
	distMatrix[i] = new float[now.ranges.size()];
float Xcross;
float Ycross;
float c1;

//ROS_INFO("populating linear matrix");
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

//ROS_INFO("parsing Hough transform matrix");
for(int j = 0; j < now.ranges.size(); j++)
{
	last = distMatrix[0][0];
	for(int i = 0; i < ANGLE_RES; i++)
	{   
		//ROS_INFO("j: %d , i: %d, Value: %f" , j, i,distMatrix[i][j]);
		// is like the last point
		if(distMatrix[i][j] > (last - THRESHOLD) && distMatrix[i][j] < (last + THRESHOLD) && !(distMatrix[i][j] < .1f) && j > 0  && (distMatrix[i][j] > 3.0f))
		{
			//new line
			if(!onAline)
			{
				//ROS_INFO("starting new line...");
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
				//ROS_INFO("continuing line...");
				skips = 0;
				linesOut.x2.back() = linearcoordX[j];
				linesOut.y2.back() = linearcoordY[j];

				lastHitX = linesOut.x2.back();
				lastHitY = linesOut.y2.back();
			
			}
				
			//ROS_INFO("moving on...");
		}
		//strayed from the line
		else
		{
			if(onAline)
			{
				//ROS_INFO("point skip");
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
					//ROS_INFO("ending line");
					 
					onAline = false;
					linesOut.x2.back() = lastHitX;
					linesOut.y2.back() = lastHitY;
					//ROS_INFO("x1: %f, y1: %f, x2: %f, y2: %f", linesOut.x1.back(), linesOut.y1.back(), linesOut.x2.back(), linesOut.y2.back()); 

					for(int k = 0; k < skips; k++)
					{
						pointsOut.x.push_back(linearcoordX[j-k]);
						pointsOut.y.push_back(linearcoordY[j-k]);
					}
				}
			}
		}
		last = distMatrix[i][j];
		
	}
	
}
info_pub1.publish(pointsOut);
info_pub2.publish(linesOut);

//ROS_INFO("cleaning...");
delete [] linearcoordY;
delete [] linearcoordX;
for(int i = 0; i < ANGLE_RES; i++)
	delete [] distMatrix[i];

ROS_INFO("NUMBER OF LINES: %d ", linesOut.x1.size());
ros::spinOnce();

}



//end your methadology


  return 0;
}




//todo:: make info gathering function
void loadLaser(const sensor_msgs::LaserScan& msg)
{
	//make a deep copy
	//these are in radians
	//ROS_INFO("Sensors updated: point count = %d", msg.ranges.size());
	now.angle_min = msg.angle_min;
	now.angle_max = msg.angle_max;
	now.angle_increment = msg.angle_increment;

	//time info
	now.time_increment = msg.time_increment;
	now.scan_time = msg.scan_time;	

	now.ranges.erase(now.ranges.begin(), now.ranges.end()); 
	for(int i = 0; i < msg.ranges.size(); i++)
	{
		now.ranges.push_back(msg.ranges[i]);
	}

}

void cleanLinesOut(p2_delta::lineList* linesOut)
{
	linesOut->x1.erase(linesOut->x1.begin(), linesOut->x1.end());
	linesOut->y1.erase(linesOut->y1.begin(), linesOut->y1.end());
	linesOut->x2.erase(linesOut->x2.begin(), linesOut->x2.end());
	linesOut->y2.erase(linesOut->y2.begin(), linesOut->y2.end());
}


