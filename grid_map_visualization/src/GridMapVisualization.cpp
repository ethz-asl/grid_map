/*
 * GridMapVisualization.cpp
 *
 *  Created on: Nov 19, 2013
 *      Author: Péter Fankhauser
 *	 Institute: ETH Zurich, ANYbotics
 */

#include <grid_map_core/GridMap.hpp>
#include <grid_map_ros/GridMapRosConverter.hpp>

#include <string>
#include <vector>
#include <memory>
#include <unordered_set>

#include "grid_map_visualization/GridMapVisualization.hpp"

namespace grid_map_visualization
{

GridMapVisualization::GridMapVisualization(const std::string & parameterName)
: visualizationsParameter_(parameterName)
  // isSubscribed_(false)
{
  nodeHandle_ = std::make_shared<rclcpp::Node>("grid_map_visualization");
  factory_ = std::make_shared<VisualizationFactory>(nodeHandle_);

  RCLCPP_INFO(nodeHandle_->get_logger(), "Grid map visualization node started.");
  readParameters();

  // Unsubscribing from a topic is not yet implemented in rclcpp
  // https://answers.ros.org/question/354792/rclcpp-how-to-unsubscribe-from-a-topic/
  // TODO(marwan99): reactivate updateSubscriptionCallback once unsubscribing is implemented
  // activityCheckTimer_ = nodeHandle_->create_wall_timer(
  //   activityCheckDuration_,
  //   &GridMapVisualization::updateSubscriptionCallback,
  //   this);
  initialize();
}

GridMapVisualization::~GridMapVisualization()
{
}

rclcpp::Node::SharedPtr
GridMapVisualization::get_node_ptr()
{
  return nodeHandle_;
}

bool GridMapVisualization::readParameters()
{
  nodeHandle_->declare_parameter("grid_map_topic", std::string("/grid_map"));
  nodeHandle_->declare_parameter("activity_check_rate", 2.0);
  nodeHandle_->declare_parameter(visualizationsParameter_, std::vector<std::string>());

  double activityCheckRate;

  nodeHandle_->get_parameter("grid_map_topic", mapTopic_);
  nodeHandle_->get_parameter("activity_check_rate", activityCheckRate);

  // activityCheckDuration_.fromSec(1.0 / activityCheckRate);
  // ROS_ASSERT(!activityCheckDuration_.isZero());

  // Configure the visualizations from a configuration stored on the parameter server.
  std::vector<std::string> config;
  if (!nodeHandle_->get_parameter(visualizationsParameter_, config)) {
    RCLCPP_WARN(
      nodeHandle_->get_logger(),
      "Could not load the visualizations configuration from parameter %s,are you sure it"
      "was pushed to the parameter server? Assuming that you meant to leave it empty.",
      visualizationsParameter_.c_str());
    return false;
  }

  std::unordered_set<std::string> config_check;

  // Iterate over all visualizations (may be just one),
  for (auto name : config) {
    std::string type;

    // Check for name collisions within the list itself.
    if (config_check.find(name) == config_check.end()) {
      config_check.insert(name);
    } else {
      RCLCPP_ERROR(
        nodeHandle_->get_logger(),
        "%s: A visualization with the name '%s' already exists.",
        visualizationsParameter_.c_str(), namei.c_str());
      return false;
    }

    nodeHandle_->declare_parameter(name + ".type");
    try {
      if (nodeHandle_->get_parameter(name + ".type", type)) {
        RCLCPP_ERROR(
          nodeHandle_->get_logger(),
          "%s: Could not add a visualization because no type was given",
          name.c_str());
        return false;
      }
    } catch (const rclcpp::ParameterTypeException & e) {
      RCLCPP_ERROR(
        nodeHandle_->get_logger(),
        "%s: Could not add %s visualization, because the %s.type parameter is not a string.",
        visualizationsParameter_.c_str(), name.c_str(), name.c_str());
      return false;
    }

    // Make sure the visualization has a valid type.
    if (!factory_->isValidType(type)) {
      RCLCPP_ERROR(
        nodeHandle_->get_logger(),
        "Could not find visualization of type '%s'.",
        type.c_str());
      return false;
    }

    auto visualization = factory_->getInstance(name, type);
    visualization->readParameters();
    visualizations_.push_back(visualization);
    RCLCPP_INFO(
      nodeHandle_->get_logger(), "%s: Configured visualization of type '%s' with name '%s'.",
      visualizationsParameter_.c_str(), type.c_str(), name.c_str());
  }
  return true;
}

bool GridMapVisualization::initialize()
{
  for (auto visualization : visualizations_) {
    visualization->initialize();
  }

  // Unsubscribing from a topic is not yet implemented in rclcpp
  // https://answers.ros.org/question/354792/rclcpp-how-to-unsubscribe-from-a-topic/
  // TODO(marwan99): reactivate updateSubscriptionCallback once unsubscribing is implemented
  // updateSubscriptionCallback(ros::TimerEvent());
  mapSubscriber_ = nodeHandle_->create_subscription<grid_map_msgs::msg::GridMap>(
    mapTopic_, rclcpp::SystemDefaultsQoS(),
    std::bind(&GridMapVisualization::callback, this, std::placeholders::_1));

  RCLCPP_INFO(nodeHandle_->get_logger(), "Grid map visualization initialized.");
  return true;
}

// void GridMapVisualization::updateSubscriptionCallback(const ros::TimerEvent & timerEvent)
// {
//   bool isActive = false;

//   for (auto & visualization : visualizations_) {
//     if (visualization->isActive()) {
//       isActive = true;
//       break;
//     }
//   }

//   if (!isSubscribed_ && isActive) {
//     mapSubscriber_ = nodeHandle_->create_subscription<grid_map_msgs::msg::GridMap>(
//       mapTopic_, rclcpp::SystemDefaultsQoS(),
//       std::bind(&GridMapVisualization::callback, this, _1));

//     isSubscribed_ = true;
//     RCLCPP_DEBUG(nodeHandle_->get_logger(),"Subscribed to grid map at '%s'.", mapTopic_.c_str());
//   }
//   if (isSubscribed_ && !isActive) {
//     mapSubscriber_.shutdown();
//     isSubscribed_ = false;
//     RCLCPP_DEBUG(nodeHandle_->get_logger(),"Cancelled subscription to grid map.");
//   }
// }￼

void GridMapVisualization::callback(const grid_map_msgs::msg::GridMap::SharedPtr message)
{
  RCLCPP_DEBUG(
    nodeHandle_->get_logger(),
    "Grid map visualization received a map (timestamp %f) for visualization.",
    rclcpp::Time(message->header.stamp).seconds());
  grid_map::GridMap map;
  grid_map::GridMapRosConverter::fromMessage(*message, map);

  for (std::size_t i = 0; i < visualizations_.size(); i++) {
    visualizations_[i]->visualize(map);
  }
}

}  // namespace grid_map_visualization
