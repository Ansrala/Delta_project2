

#include "ros/ros.h"
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
  ros::Publisher cmd_vel_pub = n.advertise<geometry_msgs::Twist>("cmd_vel", 1000);

  //the state messenger doesn't need the same resolution as the cmd::velocity channel.  
//Thus, we only publish 5 times a second.
  ros::Publisher cmd_vel_pub2 = s.advertise<geometry_msgs::Twist>("FState", 5);

ros::topic::waitForMessage<nav_msgs::Odometry>(std::string("odom"), n,ros::Duration(30));
    geometry_msgs::Twist msg;


   //we listen a little faster than we publish, since we don't know when it will change
  	ros::Subscriber suba = ls.subscribe("worldinfoObstacles", 10, checkObstacleChange);
	ros::Subscriber subb = rs.subscribe("worldinfoWalls", 10, checkWallChange);
  

   
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

/*
geometry_msgs::Twist avoidObstacle()
{

//assume currstate will have local location for obstacles (x,y,r)
//know bounds for walls (x1, y1) (x2, y2)
//assume floats
}

geometry_msgs::Twist passThroughDoor()
{

//assume currstate will have local location for obstacles (x,y,r)
//know bounds for walls (x1, y1) (x2, y2)
//assume floats
}

geometry_msgs::Twist wander()
{

}
*/

