//sensor node
//interprets data from sensors
//posts them to "worldinfo" topic

#define ANGLE_RES 7
#define OB_SIZE 7
#define THRESHOLD .02f
#define MIN_POINT_COUNT 5
#define PI 3.14592f

#include "ros/ros.h"
#include <geometry_msgs/Twist.h>
#include <nav_msgs/Odometry.h>
#include <string>
#include <std_msgs/String.h>
#include <sensor_msgs/LaserScan.h>
#include <stdlib.h>
#include <cmath>
#include <iostream>
#include <vector>

//our custom messages
#include <p2_delta/lineList.h>
#include <p2_delta/pointList.h>


using namespace std;

sensor_msgs::LaserScan now;

void loadLaser(const sensor_msgs::LaserScan& msg);

void checkStateChange(const std_msgs::String& msg);

void cleanLinesOut(p2_delta::lineList &linesOut, p2_delta::pointList &pointsOut);

int main(int argc, char **argv)
{
  ros::init(argc, argv, "p2_delta_sensors");
  ros::NodeHandle n;
  ros::NodeHandle s;
  ros::NodeHandle talk;
  ros::NodeHandle sense;
  ros::Publisher cmd_vel_pub = n.advertise<geometry_msgs::Twist>("cmd_vel", 1000);


//ros::topic::waitForMessage<nav_msgs::Odometry>(string("odom"), n,ros::Duration(30));
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
   ros::Publisher info_pub1 = talk.advertise<p2_delta::pointList>("worldinfoPoints", 50);
   ros::Publisher info_pub2 = talk.advertise<p2_delta::lineList>("worldinfoLines", 50);

   p2_delta::lineList linesOut;
   p2_delta::pointList pointsOut;
   
//begin your methodology


int testCount = 0;
while(ros::ok())
{

float* linearcoordX = new float[now.ranges.size()];
float* linearcoordY = new float[now.ranges.size()];

cleanLinesOut(linesOut, pointsOut);

//convert to linear coords here
//ROS_INFO("making linear matrix");
for (int i = 0; i < now.ranges.size(); i++)
{
	linearcoordX[i] = now.ranges[i] * cos(now.angle_min + i * now.angle_increment);
	linearcoordY[i] = now.ranges[i] * sin(now.angle_min + i * now.angle_increment);

}

//slope of i and i+1
//float slope;
bool stillwall = false;
float distanceSquared;
int tempPointCount = 0;
int skippedPointCount = 0;
float currSlope = 0;
float nextSlope = 0;
float endpoint[2];
float startpoint[2];

for (int i = 0; i < now.ranges.size(); i++)
{
ROS_INFO("i: %d",i);
	//checking for bad points
	if( now.ranges[i] < 0.01 || now.ranges[i] >4){
		skippedPointCount++;
		tempPointCount++;
		continue;
	}
	//not first point
	if(i > 0 )
	{	

		//check distance between points
		distanceSquared = pow(linearcoordX[i]-linearcoordX[i-(1+skippedPointCount)],2) + pow(linearcoordY[i] - linearcoordY[i-(1+skippedPointCount)],2);
	    if(distanceSquared < pow(0.01 * (skippedPointCount+1),2))
		{
			//new wall
			if(!stillwall)
			{
				//if current point is still in range (accounting for skips...)
				if(tempPointCount < MIN_POINT_COUNT + skippedPointCount){
					tempPointCount++;
					//if we are ready to check slope, log slope
					if(tempPointCount == MIN_POINT_COUNT + skippedPointCount){
						currSlope = (linearcoordY[i] - linearcoordY[i-tempPointCount])/(linearcoordX[i] - linearcoordX[i-tempPointCount]);
						stillwall = true;
						endpoint[0] = linearcoordX[i];
						endpoint[1] = linearcoordY[i];
						startpoint[0] = linearcoordX[i-tempPointCount];
						startpoint[1] = linearcoordY[i-tempPointCount];
					}
				}
			}
			//adding to wall
			else
			{
				//test the slope against the slope for the new point
				nextSlope = (linearcoordY[i] - endpoint[1])/(linearcoordX[i] - endpoint[0]);
				if(nextSlope > (currSlope - THRESHOLD) && nextSlope < (currSlope + THRESHOLD))
				{
					endpoint[0] = linearcoordX[i];		
					endpoint[1] = linearcoordY[i];
				}
				else
				{
					stillwall = false;
					linesOut.x1.push_back(startpoint[0]);
					linesOut.y1.push_back(startpoint[1]);
					linesOut.x2.push_back(endpoint[0]);
					linesOut.y2.push_back(endpoint[1]);
				}
			}

		}
		else
		{
			if(stillwall)
			{
				linesOut.x1.push_back(startpoint[0]);
				linesOut.y1.push_back(startpoint[1]);
				linesOut.x2.push_back(endpoint[0]);
				linesOut.y2.push_back(endpoint[1]);
			}
			stillwall = false;

			//since this point is not a wall, it is an obstacle
			pointsOut.x.push_back(linearcoordX[i]);
			pointsOut.y.push_back(linearcoordY[i]);
		}
		tempPointCount = 0;
		skippedPointCount = 0;
			
		

	}
		//record slope
		//end conversion

}

info_pub1.publish(linesOut);
info_pub2.publish(pointsOut);


//ROS_INFO("ENDING LOOP Count: %d", testCount);
//testCount++;
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

void cleanLinesOut(p2_delta::lineList &linesOut, p2_delta::pointList &pointsOut)
{
	linesOut.x1.erase(linesOut.x1.begin(), linesOut.x1.end());
	linesOut.y1.erase(linesOut.y1.begin(), linesOut.y1.end());
	linesOut.x2.erase(linesOut.x2.begin(), linesOut.x2.end());
	linesOut.y2.erase(linesOut.y2.begin(), linesOut.y2.end());

	pointsOut.x.erase(pointsOut.x.begin(), pointsOut.x.end());
	pointsOut.y.erase(pointsOut.y.begin(), pointsOut.y.end());
}


