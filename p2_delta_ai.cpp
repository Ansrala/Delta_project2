

#include "ros/ros.h"
#include "serializer/SensorState.h"
#include <geometry_msgs/Twist.h>
#include <nav_msgs/Odometry.h>
#include <string>
#include <std_msgs/String.h>

//our custom messages
#include <p2_delta/lineList.h>
#include <p2_delta/pointList.h>


using namespace std;

p2_delta::lineList walls;
p2_delta::pointList obstacles;


string currState;

void checkSensorChange(const serializer::SensorState& msg);
void checkObstacleChange(const p2_delta::pointList& msg);
//geometry_msgs::Twist passThroughDoor();
geometry_msgs::Twist avoidObstacle();
geometry_msgs::Twist wander();
float getValue();
void checkWallChange(const p2_delta::lineList& msg);

int main(int argc, char **argv)
{
  ros::init(argc, argv, "p2_delta_ai");
  ros::NodeHandle n;
  ros::NodeHandle s;
  ros::NodeHandle ls;
  ros::NodeHandle rs;
  ros::NodeHandle ss;
  ros::Publisher cmd_vel_pub = n.advertise<geometry_msgs::Twist>("cmd_vel", 1000);

  geometry_msgs::Twist output;

  //the state messenger doesn't need the same resolution as the cmd::velocity channel.  
//Thus, we only publish 5 times a second.
  ros::Publisher cmd_vel_pub2 = s.advertise<geometry_msgs::Twist>("FState", 5);

ros::topic::waitForMessage<nav_msgs::Odometry>(std::string("odom"), n,ros::Duration(30));
    geometry_msgs::Twist msg;


   //we listen a little faster than we publish, since we don't know when it will change
  	ros::Subscriber suba = ls.subscribe("worldinfoPoints", 10, checkObstacleChange);
	ros::Subscriber subb = rs.subscribe("worldinfoLines", 10, checkWallChange);
        ros::Subscriber subc = ss.subscribe("serializer_node/sensors", 10, checkSensorChange);

   
//begin your methodology

while(ros::ok())
{
	//testing stuff
	output = wander(); //avoidObstacle();
/*
	if()  //safety dance
	{}
	else if() //door...stuff
	{}
*/
	//etc
	cmd_vel_pub.publish(output);
	ros::spinOnce();
}


//end your methadology


  return 0;
}


//grab the current State
void checkObstacleChange(const p2_delta::pointList& msg)
{
	for(int i = 0; i < msg.x.size(); i++)
	{
		obstacles.x.push_back(msg.x[i]);
		obstacles.y.push_back(msg.y[i]);
		//obstacles.z.push_back(msg.z[i]);
		obstacles.radius.push_back(msg.radius[i]);
	}
}

//grab the current State
void checkWallChange(const p2_delta::lineList& msg)
{
  	for(int i = 0; i < msg.x1.size(); i++)
	{
		walls.x1.push_back(msg.x1[i]);
		walls.y1.push_back(msg.y1[i]);
		//walls.z1.push_back(msg.z1[i]);

		walls.x2.push_back(msg.x2[i]);
		walls.y2.push_back(msg.y2[i]);
		//walls.z2.push_back(msg.z2[i]);
	}
}

void checkSensorChange(const serializer::SensorState& msg)
{
  
}

geometry_msgs::Twist avoidObstacle()
{

geometry_msgs::Twist msg;
//float distance;
//distance = sqrt((pow(obstacleX,2) + pow(obstacleY,2)));
bool wallToLeft = false;
bool wallInFront = false;
for(int i = 0; i < walls.x1.size();i++){
	if(walls.x1[i] < 0 && walls.x2[i] < 0)
		wallToLeft = true;
	if(walls.x1[i] > 20 && walls.x2[i]<0)
		wallInFront = true;
}
  
		if(obstacles.y[0] > 0)
		{
		 	if(obstacles.x[0] > -16 && obstacles.x[0] < 12){
				
				//veer right
				msg.linear.x = 0.25;
				msg.angular.z = -0.25;
			}
			else if( obstacles.x[0] > 12 && obstacles.x[0] < 16){
				//veer left slightly
				msg.linear.x = 0.25;
				msg.angular.z = 0.1;
			}
			else
				msg.linear.x = 0.25;	
		}
		else if(!wallInFront){
			if(obstacles.x[0] > -16 && obstacles.x[0] < 12){
				//veer left
				msg.linear.x = 0.25;
				msg.angular.z = 0.25;

			}
			else if( obstacles.x[0] > 12 && obstacles.x[0] < 16){
				//veer right slightly
				msg.linear.x = 0.25;
				msg.angular.z = -0.1;
			}
			else
				msg.linear.x = 0.25;
		}
		else {
			//veer right
			msg.linear.x = 0.25;
			msg.angular.z = -0.50;
		}

	
	return msg;

}

//geometry_msgs::Twist passThroughDoor()
//{

//assume currstate will have local location for obstacles (x,y,r)
//know bounds for walls (x1, y1) (x2, y2)
//assume floats
//}

geometry_msgs::Twist wander()
{

  geometry_msgs::Twist msg;
  float value = getValue();
  if(value<=1.75)//I'm too close to the wall
  {
    msg.linear.x = .25;
    msg.angular.z = -.15;
    //veer to the right
  }
  else if(value>=1.25)//I'm too far from the wall
  {
    msg.linear.x = .25;
    msg.angular.z = .15;
    //veer to the left
  }
  else
  {
    msg.linear.x = .25;
    msg.angular.z = 0;
    //drive straight 
  }

  return msg;
}

float getValue()
{
  serializer::SensorState msg;
  for(int i = 0; i <=5; i++)
  {
    if(msg.name[i] == "left_ir")
    {
      return msg.value[i];
    }
  }
}





