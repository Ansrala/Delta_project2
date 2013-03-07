

#include "ros/ros.h"
#include "serializer/SensorState.h"
#include <geometry_msgs/Twist.h>
#include <nav_msgs/Odometry.h>
#include <string>
#include <std_msgs/String.h>


//our custom messages
#include <Delta_project2/lineList.h>
#include <Delta_project2/pointList.h>


using namespace std;

string currState;

void checkObstacleChange(const Delta_project2::pointList& msg);

void checkWallChange(const Delta_project2::lineList& msg);

int main(int argc, char **argv)
{
  ros::init(argc, argv, "p2_delta_ai");
  ros::NodeHandle n;
  ros::NodeHandle s;
  ros::NodeHandle ls;
  ros::NodeHandle rs;
  ros::NodeHandle ss;
  ros::Publisher cmd_vel_pub = n.advertise<geometry_msgs::Twist>("cmd_vel", 1000);

  //the state messenger doesn't need the same resolution as the cmd::velocity channel.  
//Thus, we only publish 5 times a second.
  ros::Publisher cmd_vel_pub2 = s.advertise<geometry_msgs::Twist>("FState", 5);

ros::topic::waitForMessage<nav_msgs::Odometry>(std::string("odom"), n,ros::Duration(30));
    geometry_msgs::Twist msg;


   //we listen a little faster than we publish, since we don't know when it will change
  	ros::Subscriber suba = ls.subscribe("worldinfoObstacles", 10, checkObstacleChange);
	ros::Subscriber subb = rs.subscribe("worldinfoWalls", 10, checkWallChange);
        ros::Subscriber subc = ss.subscribe("sensors", 10, checkSensorChange);

   
//begin your methodology
/*
while()
{
	//testing stuff

	if()  //safety dance
	{}
	else if() //door...stuff
	{}
	//etc
}
*/

//end your methadology


  return 0;
}


//grab the current State
void checkObstacleChange(const Delta_project2::pointList& msg)
{
  //ROS_INFO("Current State: [%s]", msg.data.c_str());
  //currState = msg.data;
}

//grab the current State
void checkWallChange(const Delta_project2::lineList& msg)
{
  //ROS_INFO("Current State: [%s]", msg.data.c_str());
  //currState = msg.data;
}

void checkSensorChange(const Serializer::SensorState& msg)
{
  
}

geometry_msgs::Twist avoidObstacle()
{

//assume currstate will have local location for obstacles (x,y,r)
//know bounds for walls (x1, y1) (x2, y2)
//assume floats
geometry_msgs::Twist msg;
//float distance;
//distance = sqrt((pow(obstacleX,2) + pow(obstacleY,2)));
	//if(/*wall exists*/ x1<0 && x2<0)//wall is entirely to left of robot
	//{
		if(obstacleY > 0)
		{
		 	if(obstacleX > -16 && obstacleX < 12){
				
				//veer right
				msg.linear.x = 0.25;
				msg.angular.z = -0.25;
			}
			else if( obstacleX > 12 && obstacleX < 16){
				//veer left slightly
				msg.linear.x = 0.25;
				msg.angular.z = 0.1;
			}
			else
				msg.linear.x = 0.25;	
		}
		else if(/*no Wall infront*/){
			if(obstacleX > -16 && obstacleX < 12){
				//veer left
				msg.linear.x = 0.25;
				msg.angular.z = 0.25;

			}
			else if( obstacleX > 12 && obstacleX < 16){
				//veer right slightly
				msg.linear.x = 0.25;
				msg.angular.z = -0.1;
			}
			else
				msg.linear.x = 0.25;
		}
		else /*wall in front*/{
			//veer right
			msg.linear.x = 0.25;
			msg.angular.z = -0.50;
		}

	//}
	return msg;

}

geometry_msgs::Twist passThroughDoor()
{

//assume currstate will have local location for obstacles (x,y,r)
//know bounds for walls (x1, y1) (x2, y2)
//assume floats
}

geometry_msgs::Twist wander()
{
  geometry_msgs::Twist msg;
  float32 value = getValue();
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

float32 getValue()
{
  serializer::sensor msg;
  for(int i = 0; msg <=5; i++)
  {
    if(msg.name[i] == "left_ir")
    {
      return msg.value[i];
    }
  }
}





