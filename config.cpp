#include "config.hpp"
#include "external/json.hpp"

#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

Config ConfigService::config_;

void ConfigService::init(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Failed to open config file");

    json j;
    file >> j;

    // --- Discord ---
    // (loaded separately via DiscordConfig::loadDiscordConfig, nothing here)

    // --- History ---
    config_.history.temp_duration = j["history"]["temp_duration"];

    // --- Line control ---
    config_.line_control.angle_gain     = j["line_control"]["angle_gain"];
    config_.line_control.curvature_gain = j["line_control"]["curvature_gain"];
    config_.line_control.offset_gain    = j["line_control"]["offset_gain"];
    config_.line_control.steps_count    = j["line_control"]["steps_count"];
    config_.line_control.weight_center  = j["line_control"]["weight_center"];
    config_.line_control.weight_sigma   = j["line_control"]["weight_sigma"];

    // --- Modules ---
    config_.modules.debug            = j["modules"]["debug"];
    config_.modules.discord          = j["modules"]["discord"];
    config_.modules.driving          = j["modules"]["driving"];
    config_.modules.logging          = j["modules"]["logging"];
    config_.modules.mirroring        = j["modules"]["mirroring"];
    config_.modules.steering_counter = j["modules"]["steering_counter"];
    config_.modules.vision           = j["modules"]["vision"];

    // --- Motor ---
    config_.motor.default_speed  = j["motor"]["default_speed"];
    config_.motor.max_speed      = j["motor"]["max_speed"];
    config_.motor.min_speed      = j["motor"]["min_speed"];
    config_.motor.steering_count = j["motor"]["steering_count"];

    // --- Perspective ---
    config_.perspective.enabled                = j["perspective"]["enabled"];
    config_.perspective.trapezoid_bottom_width = j["perspective"]["trapezoid_bottom_width"];
    config_.perspective.trapezoid_top_width    = j["perspective"]["trapezoid_top_width"];
    config_.perspective.vanishing_y            = j["perspective"]["vanishing_y"];

    // --- PID ---
    config_.pid.int_limit = j["pid"]["int_limit"];
    config_.pid.kd        = j["pid"]["kd"];
    config_.pid.ki        = j["pid"]["ki"];
    config_.pid.kp        = j["pid"]["kp"];

    // --- Quadrant ---
    config_.quadrant.num      = j["quadrant"]["num"];
    config_.quadrant.offset_y = j["quadrant"]["offset_y"];
    config_.quadrant.spacing_y= j["quadrant"]["spacing_y"];

    // --- Timing ---
    config_.timing.backtrack_duration_ms   = j["timing"]["backtrack_duration_ms"];
    config_.timing.button_debounce_ms      = j["timing"]["button_debounce_ms"];
    config_.timing.button_poll_interval_ms = j["timing"]["button_poll_interval_ms"];
    config_.timing.max_system_runtime_min  = j["timing"]["max_system_runtime_min"];
    config_.timing.max_wait_time_s         = j["timing"]["max_wait_time_s"];
    config_.timing.robot_loop_interval_ms  = j["timing"]["robot_loop_interval_ms"];
    config_.timing.sleep_time_s            = j["timing"]["sleep_time_s"];

    // --- Vision ---
    config_.vision.color_threshold   = j["vision"]["color_threshold"];
    config_.vision.max_test_attempts = j["vision"]["max_test_attempts"];
    config_.vision.min_line_area     = j["vision"]["min_line_area"];
    config_.vision.threshold_max     = j["vision"]["threshold_max"];
    config_.vision.threshold_min     = j["vision"]["threshold_min"];

    // --- Toggles ---
    config_.modules.toggleTable = {
        {'0', &config_.modules.driving,          "Driving"},
        {'1', &config_.modules.vision,           "Vision"},
        {'2', &config_.modules.discord,          "Discord"},
        {'3', &config_.modules.logging,          "Logging"},
        {'4', &config_.modules.debug,            "Debug"},
        {'5', &config_.modules.mirroring,        "Mirroring"},
        {'6', &config_.modules.steering_counter, "Steering Counter"}
    };
}

Config& ConfigService::get()
{
    return config_;
}