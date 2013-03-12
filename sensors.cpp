//sensor node
//interprets data from sensors
//posts them to "worldinfo" topic

#define ANGLE_RES 7
#define OB_SIZE 7
#define THRESHOLD 1.0f
#define MIN_POINT_COUNT 20
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
  ros::NodeHandle nh;
  //ros::NodeHandle s;
  ros::NodeHandle talk1;
  ros::NodeHandle talk2;
  ros::NodeHandle sense;
 // ros::Publisher cmd_vel_pub = n.advertise<geometry_msgs::Twist>("cmd_vel", 1000);

  ros::Rate loop_rate(10);

//test
ros::Publisher pub = nh.advertise<std_msgs::String>("topic_name", 5);
std_msgs::String str;
str.data = "hello world";
pub.publish(str);



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
   ros::Publisher info_pub1 = talk1.advertise<p2_delta::pointList>("worldinfoPoints", 50);
   ros::Publisher info_pub2 = talk2.advertise<p2_delta::lineList>("worldinfoLines", 50);

   
   
//begin your methodology


int testCount = 0;
while(ros::ok())
{

p2_delta::lineList linesOut;
p2_delta::pointList pointsOut;

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
float obstacleStart[2];
float obstacleEnd[2];
int obstaclePoints = 0;

for (int i = 0; i < now.ranges.size(); i++)
{
//ROS_INFO("i: %d",i);
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
//ROS_INFO("i GReater then 1");
		distanceSquared = pow(linearcoordX[i]-linearcoordX[i-(1+skippedPointCount)],2) + pow(linearcoordY[i] - linearcoordY[i-(1+skippedPointCount)],2);
	    if(distanceSquared < pow(0.01 * (skippedPointCount+1),2))
		{
			//ROS_INFO("distancesquared less then blah balh");
			//new wall
			if(!stillwall)
			{
				//ROS_INFO("not still wall");
				//if current point is still in range (accounting for skips...)
				if(tempPointCount < MIN_POINT_COUNT + skippedPointCount)
				{

					tempPointCount++;
				//	ROS_INFO("tempCount: %d, Minpoint: %d",tempPointCount,MIN_POINT_COUNT + skippedPointCount);
					//if we are ready to check slope, log slope
					if(tempPointCount == MIN_POINT_COUNT + skippedPointCount)
					{
					//	ROS_INFO("checking slope");
						currSlope = (linearcoordY[i] - linearcoordY[i-tempPointCount])/(linearcoordX[i] - linearcoordX[i-tempPointCount]);
						stillwall = true;
						endpoint[0] = linearcoordX[i];
						endpoint[1] = linearcoordY[i];
						startpoint[0] = linearcoordX[i-tempPointCount];
						startpoint[1] = linearcoordY[i-tempPointCount];
						tempPointCount = 0;
						skippedPointCount = 0;
					}
				}
			}
			//adding to wall
			else
			{
				//ROS_INFO("still wall");
				//test the slope against the slope for the new point
				nextSlope = (linearcoordY[i] - endpoint[1])/(linearcoordX[i] - endpoint[0]);
				if(nextSlope > (currSlope - THRESHOLD) && nextSlope < (currSlope + THRESHOLD))
				{
					endpoint[0] = linearcoordX[i];		
					endpoint[1] = linearcoordY[i];
				}
				else
				{
			//		ROS_INFO("wall end");
					stillwall = false;
					linesOut.x1.push_back(startpoint[0]);
					linesOut.y1.push_back(startpoint[1]);
					linesOut.x2.push_back(endpoint[0]);
					linesOut.y2.push_back(endpoint[1]);
					tempPointCount = 0;
					skippedPointCount = 0;
				}
			}

		}
		else
		{
		//	ROS_INFO("distancesquared greater then blah balh");
			tempPointCount++;
			if(stillwall)
			{
			//	ROS_INFO("stillwall");
				linesOut.x1.push_back(startpoint[0]);
				linesOut.y1.push_back(startpoint[1]);
				linesOut.x2.push_back(endpoint[0]);
				linesOut.y2.push_back(endpoint[1]);
			}
			stillwall = false;
			tempPointCount = 0;
			skippedPointCount = 0;
			//since this point is not a wall, it is an obstacle
			//but is it part of the same obstacle?
			if(obstaclePoints == 0)
			{
				//new obstacle
				obstaclePoints++;
				obstacleStart[0] = linearcoordX[i];
				obstacleStart[1] = linearcoordY[i];
				obstacleEnd[0] = linearcoordX[i];
				obstacleEnd[1] = linearcoordY[i];
				tempPointCount = 0;
				skippedPointCount = 0;
			}
			else
			{
				distanceSquared = pow(linearcoordX[i]-linearcoordX[i-(1+skippedPointCount)],2) + pow(linearcoordY[i] - linearcoordY[i-(1+skippedPointCount)],2);
				if(distanceSquared < pow(0.01 * (skippedPointCount+1),2))
				{
					obstacleEnd[0] = linearcoordX[i];
					obstacleEnd[1] = linearcoordY[i];
					obstaclePoints++;
				}
				else
				{
					//push average location
					pointsOut.x.push_back((obstacleEnd[0] + obstacleStart[0])/2);
					pointsOut.y.push_back((obstacleEnd[1] + obstacleStart[1])/2);
					obstaclePoints = 0;
				}
				tempPointCount = 0;
				skippedPointCount = 0;
			}
				
			
		}
		
			
		

	}
		//record slope
		//end conversion

}
ROS_INFO("lines out size: %d",linesOut.x1.size());
info_pub2.publish(linesOut);
info_pub1.publish(pointsOut);


//ROS_INFO("ENDING LOOP Count: %d", testCount);
//testCount++;
ros::spinOnce();
loop_rate.sleep();

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


