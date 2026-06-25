#include <Arduino.h>
#include "ros.h"
#include "kinematik.h"
#include "pid.h"
#include "odometri.h"
#include "sabitler.h"

#include <micro_ros_platformio.h>
#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <rmw_microros/rmw_microros.h>

#include <geometry_msgs/msg/twist.h>
#include <nav_msgs/msg/odometry.h>
#include <sensor_msgs/msg/joint_state.h>

// --- micro-ROS ENTITY'LER ---
static rcl_subscription_t cmd_vel_subscriber;
static rcl_publisher_t    odom_publisher;
static rcl_publisher_t    joint_state_publisher;

static geometry_msgs__msg__Twist     cmd_vel_msg;
static nav_msgs__msg__Odometry       odom_msg;
static sensor_msgs__msg__JointState  joint_state_msg;

static rclc_support_t   support;
static rcl_allocator_t  allocator;
static rcl_init_options_t init_options;
static rcl_node_t       node;
static rcl_timer_t      odom_timer;
static rcl_timer_t      joint_state_timer;
static rclc_executor_t  executor;

// Tekerlek toplam dönüş açıları
static float toplamRadSag = 0.0;
static float toplamRadSol = 0.0;

AgentDurum agentDurum;

#define RCCHECK(fn) { rcl_ret_t rc = fn; if(rc != RCL_RET_OK) return false; }
#define EXECUTE_EVERY_N_MS(MS, X) do { \
  static volatile int64_t init = -1; \
  if (init == -1) { init = uxr_millis(); } \
  if (uxr_millis() - init > MS) { X; init = uxr_millis(); } \
} while (0)

// =========== CALLBACK'LER ===========

static void cmd_vel_callback(const void* msgin) {
  const geometry_msgs__msg__Twist* msg = (const geometry_msgs__msg__Twist*)msgin;
  kinematikHesapla(msg->linear.x, msg->angular.z);
  pidResetle();
}

static void odom_timer_callback(rcl_timer_t* timer, int64_t) {
  if (timer == NULL) return;

  static char frame_odom[] = "odom";
  static char frame_base[] = "base_link";
  odom_msg.header.frame_id.data = frame_odom;
  odom_msg.header.frame_id.size = strlen(frame_odom);
  odom_msg.header.frame_id.capacity = strlen(frame_odom) + 1;
  odom_msg.child_frame_id.data = frame_base;
  odom_msg.child_frame_id.size = strlen(frame_base);
  odom_msg.child_frame_id.capacity = strlen(frame_base) + 1;

  float cy = cos(odomTheta * 0.5);
  float sy = sin(odomTheta * 0.5);

  odom_msg.header.stamp.sec = (int32_t)(millis() / 1000);
  odom_msg.header.stamp.nanosec = (uint32_t)((millis() % 1000) * 1000000);
  odom_msg.pose.pose.position.x = odomX;
  odom_msg.pose.pose.position.y = odomY;
  odom_msg.pose.pose.position.z = 0.0;
  odom_msg.pose.pose.orientation.z = sy;
  odom_msg.pose.pose.orientation.w = cy;
  odom_msg.twist.twist.linear.x = vRobot;
  odom_msg.twist.twist.angular.z = wRobot;

  rcl_publish(&odom_publisher, &odom_msg, NULL);
}

static void joint_state_timer_callback(rcl_timer_t* timer, int64_t) {
  if (timer == NULL) return;

  // Tekerlek açılarını biriktir
  toplamRadSag += gercekRadsSag * 0.05;  // 50ms periyot
  toplamRadSol += gercekRadsSol * 0.05;

  joint_state_msg.header.stamp.sec = (int32_t)(millis() / 1000);
  joint_state_msg.header.stamp.nanosec = (uint32_t)((millis() % 1000) * 1000000);

  joint_state_msg.position.data[0] = toplamRadSol;
  joint_state_msg.position.data[1] = toplamRadSag;
  joint_state_msg.velocity.data[0] = gercekRadsSol;
  joint_state_msg.velocity.data[1] = gercekRadsSag;

  rcl_publish(&joint_state_publisher, &joint_state_msg, NULL);
}

// =========== ENTITY OLUSTUR / YOKET ===========

static bool entities_olustur() {
  allocator = rcl_get_default_allocator();

  // Domain ID 17 ayarla
  init_options = rcl_get_zero_initialized_init_options();
  RCCHECK(rcl_init_options_init(&init_options, allocator));
  RCCHECK(rcl_init_options_set_domain_id(&init_options, GEZ_DOMAIN_ID));
  RCCHECK(rclc_support_init_with_options(&support, 0, NULL, &init_options, &allocator));

  RCCHECK(rclc_node_init_default(&node, "gez", "", &support));

  // Publishers ve subscriber
  RCCHECK(rclc_subscription_init_default(&cmd_vel_subscriber, &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist), "cmd_vel"));
  RCCHECK(rclc_publisher_init_default(&odom_publisher, &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(nav_msgs, msg, Odometry), "odom"));
  RCCHECK(rclc_publisher_init_default(&joint_state_publisher, &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, JointState), "joint_states"));

  // Timer'lar (50ms)
  RCCHECK(rclc_timer_init_default(&odom_timer, &support,
    RCL_MS_TO_NS(50), odom_timer_callback));
  RCCHECK(rclc_timer_init_default(&joint_state_timer, &support,
    RCL_MS_TO_NS(50), joint_state_timer_callback));

  // Executor
  RCCHECK(rclc_executor_init(&executor, &support.context, 3, &allocator));
  rclc_executor_add_subscription(&executor, &cmd_vel_subscriber, &cmd_vel_msg,
                                 &cmd_vel_callback, ON_NEW_DATA);
  rclc_executor_add_timer(&executor, &odom_timer);
  rclc_executor_add_timer(&executor, &joint_state_timer);

  // Joint state mesaj alanları
  static double position_data[2] = {0.0, 0.0};
  static double velocity_data[2] = {0.0, 0.0};
  joint_state_msg.position.data = position_data;
  joint_state_msg.position.capacity = 2;
  joint_state_msg.position.size = 2;
  joint_state_msg.velocity.data = velocity_data;
  joint_state_msg.velocity.capacity = 2;
  joint_state_msg.velocity.size = 2;

  static rosidl_runtime_c__String joint_names[2];
  static char name_sol[] = "sol_tekerlek_joint";
  static char name_sag[] = "sag_tekerlek_joint";
  joint_names[0].data = name_sol;
  joint_names[0].size = strlen(name_sol);
  joint_names[0].capacity = strlen(name_sol) + 1;
  joint_names[1].data = name_sag;
  joint_names[1].size = strlen(name_sag);
  joint_names[1].capacity = strlen(name_sag) + 1;
  joint_state_msg.name.data = joint_names;
  joint_state_msg.name.size = 2;
  joint_state_msg.name.capacity = 2;

  return true;
}

static void entities_yoket() {
  rmw_context_t* rmw_context = rcl_context_get_rmw_context(&support.context);
  rmw_uros_set_context_entity_destroy_session_timeout(rmw_context, 0);
  rcl_subscription_fini(&cmd_vel_subscriber, &node);
  rcl_publisher_fini(&odom_publisher, &node);
  rcl_publisher_fini(&joint_state_publisher, &node);
  rcl_timer_fini(&odom_timer);
  rcl_timer_fini(&joint_state_timer);
  rclc_executor_fini(&executor);
  rcl_node_fini(&node);
  rclc_support_fini(&support);
}

// =========== PUBLIC API ===========

void rosBaslat() {
  Serial.begin(921600);
  set_microros_serial_transports(Serial);
  delay(2000);
  agentDurum = BEKLEME;
}

void rosDongusu() {
  switch (agentDurum) {
    case BEKLEME:
      EXECUTE_EVERY_N_MS(500, agentDurum =
        (RMW_RET_OK == rmw_uros_ping_agent(100, 1)) ? BAGLANIYOR : BEKLEME;);
      break;

    case BAGLANIYOR:
      agentDurum = entities_olustur() ? BAGLI : KOPUK;
      if (agentDurum == KOPUK) entities_yoket();
      break;

    case BAGLI:
      EXECUTE_EVERY_N_MS(2000, agentDurum =
        (RMW_RET_OK == rmw_uros_ping_agent(100, 1)) ? BAGLI : KOPUK;);
      if (agentDurum == BAGLI) {
        rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10));
      }
      break;

    case KOPUK:
      entities_yoket();
      kinematikHesapla(0, 0);
      pidResetle();
      agentDurum = BEKLEME;
      break;
  }
}
