#include "support.h"

/* getRPY() //{ */
void mrs_odometry::getRPY(const geometry_msgs::Quaternion& q_msg_in, double& roll_out, double& pitch_out, double& yaw_out) {
    tf2::Quaternion q_tmp;
    tf2::fromMsg(q_msg_in, q_tmp);
    tf2::Matrix3x3(q_tmp).getRPY(roll_out, pitch_out, yaw_out);
}
//}

/* setRPY() //{ */
void mrs_odometry::setRPY(const double& roll_in, const double& pitch_in, const double& yaw_in, geometry_msgs::Quaternion& q_msg_out) {
  tf2::Quaternion q_tmp;
  q_tmp.setRPY(roll_in, pitch_in, yaw_in);
  q_msg_out = tf2::toMsg(q_tmp);
}
//}

/* addRPY() //{ */
void mrs_odometry::addRPY(geometry_msgs::Quaternion& q_msg, const double& roll_add, const double& pitch_add, const double& yaw_add) {
  double roll, pitch, yaw;
  getRPY(q_msg, roll, pitch, yaw);
  roll += roll_add;
  pitch += pitch_add;
  yaw += yaw_add;
  setRPY(roll, pitch, yaw, q_msg);
}
//}

  /* getYaw() //{ */
  double mrs_odometry::getYaw(tf2::Quaternion &q_tf) {
    double roll, pitch, yaw;
    tf2::Matrix3x3(q_tf).getRPY(roll, pitch, yaw);
    return yaw;
  }
  //}
  
  /* getYaw() //{ */
  double mrs_odometry::getYaw(geometry_msgs::Quaternion &q_msg) {
    tf2::Quaternion q_tf;
    tf2::fromMsg(q_msg, q_tf);
    return getYaw(q_tf);;
  }
  //}
  
  /* setYaw() //{ */
  void mrs_odometry::setYaw(geometry_msgs::Quaternion &q_msg, const double &yaw_in) {
    tf2::Quaternion q_tf;
    tf2::fromMsg(q_msg, q_tf);
    setYaw(q_tf, yaw_in);
    q_msg = tf2::toMsg(q_tf);
    q_tf.normalize();
  }
  //}
  
  /* setYaw() //{ */
  void mrs_odometry::setYaw(tf2::Quaternion &q_msg, const double &yaw_in) {
    double roll, pitch, yaw;
    tf2::Matrix3x3(q_msg).getRPY(roll, pitch, yaw);
    yaw = yaw_in;
    q_msg.setRPY(roll, pitch, yaw);
    q_msg.normalize();
  }
  //}

/* addYaw() //{ */
void mrs_odometry::addYaw(geometry_msgs::Quaternion& q_msg, const double& yaw_add) {
  double roll, pitch, yaw;
  getRPY(q_msg, roll, pitch, yaw);
  yaw += yaw_add;
  setRPY(roll, pitch, yaw, q_msg);
}
//}

/* distance() //{ */
double mrs_odometry::distance(const nav_msgs::Odometry& odom1, const nav_msgs::Odometry& odom2) {
  return std::sqrt(pow(odom1.pose.pose.position.x - odom2.pose.pose.position.x, 2) 
      + pow(odom1.pose.pose.position.y - odom2.pose.pose.position.y, 2) 
    + pow(odom1.pose.pose.position.z - odom2.pose.pose.position.z, 2) );
}
//}

/* isEqual() //{ */

bool mrs_odometry::isEqual(const char* s1, const char* s2) {
  if (std::strcmp(s1, s2)==0) {
    return true;
  } else {
    return false;
  }
}

bool mrs_odometry::isEqual(const std::string& s1, const std::string& s2) {
  return mrs_odometry::isEqual(s1.c_str(), s2.c_str());
}

bool mrs_odometry::isEqual(const std::string& s1, const char* s2) {
  return mrs_odometry::isEqual(s1.c_str(), s2);
}

bool mrs_odometry::isEqual(const char* s1, const std::string& s2) {
  return mrs_odometry::isEqual(s1, s2.c_str());
}

//}

  /* unwrapAngle() //{ */
  double mrs_odometry::unwrapAngle(const double yaw, const double yaw_previous) {
    if (yaw - yaw_previous > M_PI) {
      return yaw - 2 * M_PI;
    } else if (yaw - yaw_previous < -M_PI) {
      return yaw + 2 * M_PI;
    }
    return yaw;
  }

  //}

  /* wrapAngle() //{ */
  double mrs_odometry::wrapAngle(const double angle_in) {

    double angle_wrapped = angle_in;

    while (angle_wrapped > M_PI) {
      angle_wrapped -= 2 * M_PI;
    }

    while (angle_wrapped < -M_PI) {
      angle_wrapped += 2 * M_PI;
    }

    return angle_wrapped;
  }
  //}

  /* disambiguateAngle() //{ */
  double mrs_odometry::disambiguateAngle(const double yaw, const double yaw_previous) {
    if (yaw - yaw_previous > M_PI/2) {
      return yaw - M_PI;
    } else if (yaw - yaw_previous < -M_PI/2) {
      return yaw + M_PI;
    }
    return yaw;
  }

  //}
