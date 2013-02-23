echo "source /opt/ros/groovy/setup.bash" >> ~/.bashrc

mkdir ~/ros_pkgs

echo "export ROS_WORKSPACE=$HOME/ros_pkgs" >> ~/.bashrc

echo "export ROS_PACKAGE_PATH=\${ROS_PACKAGE_PATH}:\${ROS_WORKSPACE}" >> ~/.bashrc

echo "export P2REPO=https://github.com/Ansrala/Delta_project2.git" >> ~/.bashrc

git clone $P2REPO $ROS_WORKSPACE

rosdep update

. ~/.bashrc

svn co http://pi-robot-ros-pkg.googlecode.com/svn/trunk/serializer

roscd

rosmake serializer
