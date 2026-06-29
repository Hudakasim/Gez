#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from nav_msgs.msg import Odometry
from sensor_msgs.msg import JointState
from geometry_msgs.msg import TransformStamped
from tf2_ros import TransformBroadcaster


class OdomToTF(Node):
    def __init__(self):
        super().__init__('odom_to_tf')
        self.tf_broadcaster = TransformBroadcaster(self)

        # Odom subscriber
        self.create_subscription(Odometry, '/odom', self.odom_callback, 10)

        # JointState subscriber ve re-publisher
        self.joint_pub = self.create_publisher(JointState, '/joint_states_fixed', 10)
        self.create_subscription(JointState, '/joint_states', self.joint_callback, 10)

        self.get_logger().info('odom_to_tf basladi (timestamp duzeltme aktif)')

    def odom_callback(self, msg):
        t = TransformStamped()
        t.header.stamp = self.get_clock().now().to_msg()
        t.header.frame_id = 'odom'
        t.child_frame_id = 'base_link'
        t.transform.translation.x = msg.pose.pose.position.x
        t.transform.translation.y = msg.pose.pose.position.y
        t.transform.translation.z = msg.pose.pose.position.z
        t.transform.rotation = msg.pose.pose.orientation
        self.tf_broadcaster.sendTransform(t)

    def joint_callback(self, msg):
        # Yeni mesaj olusturup timestampi degistir
        new_msg = JointState()
        new_msg.header.stamp = self.get_clock().now().to_msg()
        new_msg.header.frame_id = msg.header.frame_id
        new_msg.name = msg.name
        new_msg.position = msg.position
        new_msg.velocity = msg.velocity
        new_msg.effort = msg.effort
        self.joint_pub.publish(new_msg)


def main():
    rclpy.init()
    node = OdomToTF()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()
