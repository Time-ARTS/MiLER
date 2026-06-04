#include "config.hpp"
#include "external/json.hpp"

#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

std::string ConfigService::path_;
Config ConfigService::config_;

void ConfigService::init(const std::string& path)
{
    path_ = path;
    reload();
}

void ConfigService::reload()
{
    if (path_.empty())
        throw std::runtime_error("ConfigService::reload called before init");

    std::ifstream file(path_);
    if (!file.is_open())
        throw std::runtime_error("Failed to open config file: " + path_);

    json j;
    file >> j;

    // --- Button ---
    config_.button.active_low = j["button"]["active_low"];
    config_.button.start_stop = j["button"]["start/stop"];

    // --- History ---
    config_.history.temp_duration = j["history"]["temp_duration"];

    // --- Keys ---
    config_.keys.error_test = j["keys"]["error_test"];
    config_.keys.perspective = j["keys"]["perspective"];
    config_.keys.quit = j["keys"]["quit"];
    config_.keys.start = j["keys"]["start"];

    // --- Line control ---
    config_.line_control.angle_gain = j["line_control"]["angle_gain"];
    config_.line_control.curvature_gain = j["line_control"]["curvature_gain"];
    config_.line_control.offset_gain = j["line_control"]["offset_gain"];
    config_.line_control.steps_count = j["line_control"]["steps_count"];
    config_.line_control.weight_center = j["line_control"]["weight_center"];
    config_.line_control.weight_sigma = j["line_control"]["weight_sigma"];

    // --- Modules ---
    config_.modules.debug = j["modules"]["debug"];
    config_.modules.driving = j["modules"]["driving"];
    config_.modules.logging = j["modules"]["logging"];
    config_.modules.mirroring = j["modules"]["mirroring"];
    config_.modules.vision = j["modules"]["vision"];

    // --- Motor ---
    config_.motor.default_speed = j["motor"]["default_speed"];
    config_.motor.max_speed = j["motor"]["max_speed"];
    config_.motor.min_speed = j["motor"]["min_speed"];
    config_.motor.steering_count = j["motor"]["steering_count"];

    // --- Motor pins ---
    config_.motor_pins.ai1 = j["motor_pins"]["ai1"];
    config_.motor_pins.ai2 = j["motor_pins"]["ai2"];
    config_.motor_pins.bi1 = j["motor_pins"]["bi1"];
    config_.motor_pins.bi2 = j["motor_pins"]["bi2"];
    config_.motor_pins.pwma_l = j["motor_pins"]["pwmA-l"];
    config_.motor_pins.pwmb_r = j["motor_pins"]["pwmB-r"];
    config_.motor_pins.stby = j["motor_pins"]["stby"];

    // --- Perspective ---
    config_.perspective.enabled = j["perspective"]["enabled"];
    config_.perspective.trapezoid_bottom_width = j["perspective"]["trapezoid_bottom_width"];
    config_.perspective.trapezoid_top_width = j["perspective"]["trapezoid_top_width"];
    config_.perspective.vanishing_y = j["perspective"]["vanishing_y"];

    // --- PID ---
    config_.pid.int_limit = j["pid"]["int_limit"];
    config_.pid.kd = j["pid"]["kd"];
    config_.pid.ki = j["pid"]["ki"];
    config_.pid.kp = j["pid"]["kp"];

    // --- Quadrant ---
    config_.quadrant.num = j["quadrant"]["num"];
    config_.quadrant.offset_y = j["quadrant"]["offset_y"];
    config_.quadrant.spacing_y = j["quadrant"]["spacing_y"];

    // --- Screen ---
    config_.screen.center_x = j["screen"]["center_x"];
    config_.screen.center_y = j["screen"]["center_y"];
    config_.screen.height = j["screen"]["height"];
    config_.screen.width = j["screen"]["width"];

    // --- Timing ---
    config_.timing.backtrack_duration_ms = j["timing"]["backtrack_duration_ms"];
    config_.timing.backtrack_wait_time_ms = j["timing"]["backtrack_wait_time_ms"];
    config_.timing.button_debounce_ms = j["timing"]["button_debounce_ms"];
    config_.timing.button_poll_interval_ms = j["timing"]["button_poll_interval_ms"];
    config_.timing.max_system_runtime_min = j["timing"]["max_system_runtime_min"];
    config_.timing.max_wait_time_s = j["timing"]["max_wait_time_s"];
    config_.timing.robot_loop_interval_ms = j["timing"]["robot_loop_interval_ms"];
    config_.timing.sleep_time_s = j["timing"]["sleep_time_s"];

    // --- Vision ---
    config_.vision.color_threshold = j["vision"]["color_threshold"];
    config_.vision.max_test_attempts = j["vision"]["max_test_attempts"];
    config_.vision.min_line_area = j["vision"]["min_line_area"];
    config_.vision.threshold_max = j["vision"]["threshold_max"];
    config_.vision.threshold_min = j["vision"]["threshold_min"];

    // --- Toggles ---
    config_.modules.toggleTable = {
        {"0", &config_.modules.driving, "Driving"},
        {"1", &config_.modules.vision, "Vision"},
        {"2", &config_.modules.logging, "Logging"},
        {"3", &config_.modules.debug, "Debug"},
        {"4", &config_.modules.mirroring, "Mirroring"},
    };
}