export PX4_ENV_PATH="/home/gi/Downloads/PX4_ENV_bak" # path to your px4 1.8
export SF="/src/Firmware"
export GAZEBO_PLUGIN_PATH=${GAZEBO_PLUGIN_PATH}:${PX4_ENV_PATH}/devel/lib/
export LAUNCH_FILE="mavros_posix_sitl.launch"
export DRONE_ITEM="iris_stereo_rgb_lidar_gps"  # can be "iris" "iris_stereo_rgb_gps" "iris_stereo_gray_gps".
#To use iris_vision, you need to copy files from GAAS. See Readme.md.

echo "COMMAND: roslaunch px4 $LAUNCH_FILE vehicle:=$DRONE_ITEM"
cd $PX4_ENV_PATH
catkin build
cd $PX4_ENV_PATH$SF
source ./Tools/setup_gazebo.bash $(pwd) $(pwd)/build/posix_sitl_default
export ROS_PACKAGE_PATH=$ROS_PACKAGE_PATH:$(pwd)
export ROS_PACKAGE_PATH=$ROS_PACKAGE_PATH:$(pwd)/Tools/sitl_gazebo:${PX4_ENV_PATH}/src/velodyne_simulator
echo "ROS_PACKAGE_PATH:"${ROS_PACKAGE_PATH}

roslaunch px4 $LAUNCH_FILE vehicle:=$DRONE_ITEM
