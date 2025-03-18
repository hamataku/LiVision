import genesis as gs
import numpy as np

from nav_msgs.msg import Odometry
from geometry_msgs.msg import TransformStamped

# ROS2
import rclpy
from rclpy.node import Node as ROSNode
import tf2_ros as tf2

# ros messages
from sensor_msgs.msg import PointCloud2, PointField

class Drone:
    def __init__(self):
        gs.init(backend=gs.gpu)

        self.drone_pos = [0.0, 0.0, 0.1]
        self.drone_vel = [0.0, 0.0, 0.0]

        self.scene = gs.Scene(
            show_viewer = False,
            viewer_options = gs.options.ViewerOptions(
                res           = (1280, 960),
                camera_pos    = (2.0, 0.0, 2.0),
                camera_lookat = (0.0, 0.0, 0.0),
                camera_fov    = 40,
                max_FPS       = 60,
            ),
            vis_options = gs.options.VisOptions(
                show_world_frame = False,
                world_frame_size = 1.0,
                show_link_frame  = False,
                show_cameras     = False,
                plane_reflection = False,
                ambient_light    = (0.1, 0.1, 0.1),
                shadow = False,
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
                fixed = True
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

        self.ros_init()
    
    def ros_init(self):
        rclpy.init()
        self.ros_node = ROSNode("genesis_simulator")
        ros2_tf_broadcaster = tf2.TransformBroadcaster(self.ros_node)
        self.publisher = self.ros_node.create_publisher(PointCloud2, "lidar", 10)

        self.msg = PointCloud2()
        self.msg.height = 1
        self.msg.fields = [
            PointField(name='x', offset=0, datatype=PointField.FLOAT32, count=1),
            PointField(name='y', offset=4, datatype=PointField.FLOAT32, count=1),
            PointField(name='z', offset=8, datatype=PointField.FLOAT32, count=1)]
        self.msg.point_step = 12
        self.msg.is_bigendian = False
        self.msg.is_dense = False
        self.msg.header.frame_id = 'map'
    
    def ros_publish(self, data):
            self.msg.header.stamp = self.ros_node.get_clock().now().to_msg()
            self.msg.width = data.shape[0]
            self.msg.row_step = self.msg.point_step * self.msg.width
            self.msg.data = np.float32(data).tostring()
            self.publisher.publish(self.msg)
        
    def run(self):
        for i in range(100000):
            if i % 2000 > 1000:
                self.drone_vel = [0.1, 0.0, 0.0]
            else:
                self.drone_vel = [-0.1, 0.0, 0.0]
            self.drone.set_velocity(self.drone_vel)
            self.drone.update()
            if i % 10 == 0:
                data = self.drone.render_sensors()
                self.ros_publish(data[0])
            
            self.scene.step()

if __name__ == '__main__':
    drone = Drone()
    drone.run()