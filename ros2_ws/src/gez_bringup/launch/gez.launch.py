import os
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, ExecuteProcess
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    # 1. micro-ROS Agent
    microros_agent = ExecuteProcess(
        cmd=[
            'ros2', 'run', 'micro_ros_agent', 'micro_ros_agent',
            'serial', '--dev', '/dev/ttyUSB0', '-b', '921600'
        ],
        output='screen'
    )

    # 2. Robot state publisher (URDF -> TF)
    gez_description_path = get_package_share_directory('gez_description')
    display_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(gez_description_path, 'launch', 'display.launch.py')
        )
    )

    # 3. Odom -> TF köprüsü
    odom_to_tf = Node(
        package='gez_bringup',
        executable='odom_to_tf',
        name='odom_to_tf',
        output='screen'
    )

    return LaunchDescription([
        microros_agent,
        display_launch,
        odom_to_tf,
    ])
