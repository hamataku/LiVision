import genesis as gs
import numpy as np

import genesis.ext.ros2.ros_core as ROS
from nav_msgs.msg import Odometry
from geometry_msgs.msg import TransformStamped

class Drone:
    def __init__(self):
        gs.init(backend=gs.gpu)

        self.drone_pos = [0.0, 0.0, 1.0]
        self.drone_vel = [0.0, 0.0, 0.0]

        self.scene = gs.Scene(
            show_viewer = True,
            viewer_options = gs.options.ViewerOptions(
                res           = (1280, 960),
                camera_pos    = (2.0, 0.0, 2.0),
                camera_lookat = (0.0, 0.0, 0.0),
                camera_fov    = 40,
                max_FPS       = 60,
            ),
            vis_options = gs.options.VisOptions(
                show_world_frame = True,
                world_frame_size = 1.0,
                show_link_frame  = False,
                show_cameras     = False,
                plane_reflection = False,
                ambient_light    = (0.1, 0.1, 0.1),
            ),
            sim_options = gs.options.SimOptions(
                dt=0.01
            ),
            renderer=gs.renderers.Rasterizer(),
        )

        plane = self.scene.add_entity(
            gs.morphs.Plane(),
        )

        self.drone = self.scene.add_entity(
            gs.morphs.EasyDrone(
                file = './drone.urdf',
                pos  = self.drone_pos,
            )
        )

        box1 = self.scene.add_entity(
            gs.morphs.Box(
                size = (0.2, 0.2, 0.6),
                pos  = (1.0, 0.0, 0.3),
                fixed = True,
            )
        )

        box2 = self.scene.add_entity(
            gs.morphs.Box(
                size = (0.2, 0.2, 0.6),
                pos  = (1.0, -1.0, 0.3),
                fixed = True,
            )
        )

        box3 = self.scene.add_entity(
            gs.morphs.Box(
                size = (0.2, 0.2, 0.6),
                pos  = (-1.0, -1.0, 0.3),
                fixed = True,
            )
        )

        self.scene.build()
    
    def ros_init(self):
        self.odom_pub = ROS.ros_node.create_publisher(Odometry, 'odom', 10)
    
    def ros_publish(self, count):
        if count % 10 == 0:
            odom_msg = Odometry()
            odom_msg.header.stamp = ROS.ros_node.get_clock().now().to_msg()
            odom_msg.header.frame_id = 'map'
            odom_msg.child_frame_id = 'drone0'
            odom_msg.pose.pose.position.x = self.drone_pos[0]
            odom_msg.pose.pose.position.y = self.drone_pos[1]
            odom_msg.pose.pose.position.z = self.drone_pos[2]
            odom_msg.pose.pose.orientation.x = 0.0
            odom_msg.pose.pose.orientation.y = 0.0
            odom_msg.pose.pose.orientation.z = 0.0
            odom_msg.pose.pose.orientation.w = 1.0
            self.odom_pub.publish(odom_msg)
        
    def run(self):
        self.ros_init()

        for i in range(100000):
            self.drone_vel = [0.1, 0.1, 0.0]
            self.scene.step()
            self.ros_publish(i)

if __name__ == '__main__':
    drone = Drone()
    drone.run()