#pragma once

// UI Layout Constants
namespace UIConfig {
    constexpr float WINDOW_WIDTH = 1200.0f;
    constexpr float WINDOW_HEIGHT = 800.0f;
    constexpr float MAP_VIEWPORT_WIDTH = 900.0f;
    constexpr float CONTROL_PANEL_WIDTH = 300.0f;
    constexpr float CONTROL_PANEL_X = 900.0f;

    constexpr float BUTTON_WIDTH = 130.0f;
    constexpr float BUTTON_HEIGHT = 28.0f;
    constexpr float BUTTON_SPACING = 32.0f;
    constexpr float COLUMN_SPACING = 5.0f;
    constexpr float BUTTON_START_Y = 190.0f;

    constexpr unsigned int PANEL_TITLE_SIZE = 20;
    constexpr unsigned int LABEL_SIZE = 14;
    constexpr unsigned int INPUT_TEXT_SIZE = 16;
    constexpr unsigned int STATS_TEXT_SIZE = 11;
    constexpr unsigned int BUTTON_TEXT_SIZE = 13;
    constexpr unsigned int LEGEND_TEXT_SIZE = 12;

    constexpr float INPUT_BOX_WIDTH = 200.0f;
    constexpr float INPUT_BOX_HEIGHT = 25.0f;
}

// Simulation Constants
namespace SimConfig {
    constexpr int MAX_ACTIVE_CARS = 100;
    constexpr int PEAK_HOUR_CAR_COUNT = 30;
    constexpr int RUSH_HOUR_CAR_COUNT = 40;
    constexpr int MULTI_CAR_SPAWN_COUNT = 20;

    constexpr float DEFAULT_SIMULATION_SPEED = 1.0f;
    constexpr float RUSH_HOUR_SIMULATION_SPEED = 0.3f;

    constexpr float ACCIDENT_DURATION_SECONDS = 180.0f;
    constexpr float PEAK_HOUR_SPEED_MULTIPLIER = 0.3f;

    constexpr float CONGESTION_MULTIPLIER = 0.3f;
    constexpr float SPAWN_INTERVAL_LOW = 2.0f;
    constexpr float SPAWN_INTERVAL_MEDIUM = 3.0f;
    constexpr float SPAWN_INTERVAL_HIGH = 5.0f;
}

// Rendering Constants
namespace RenderConfig {
    constexpr float MIN_ZOOM = 0.1f;
    constexpr float MAX_ZOOM = 5.0f;
    constexpr float ZOOM_INCREMENT = 1.1f;
    constexpr float ZOOM_DECREMENT = 0.9f;

    constexpr float NODE_RADIUS = 8.0f;
    constexpr float NODE_TEXT_SIZE = 10.0f;
    constexpr float MAX_TEXT_ZOOM = 2.0f;

    constexpr float BASE_ROAD_WIDTH = 3.0f;
    constexpr float ACCIDENT_ROAD_WIDTH = 5.0f;
    constexpr float PATH_WIDTH = 6.0f;
    constexpr float PREDICTION_OVERLAY_WIDTH = 8.0f;

    constexpr float NODE_SELECTION_RADIUS = 15.0f;
    constexpr float MIN_EDGE_LENGTH = 0.1f;

    constexpr float CAR_TRIANGLE_HEIGHT = 8.0f;
    constexpr float CAR_TRIANGLE_WIDTH = 5.0f;

    constexpr unsigned char PREDICTION_ALPHA = 180;
    constexpr unsigned char PATH_ALPHA = 180;
}

// Traffic Level Constants
namespace TrafficConfig {
    constexpr float FREE_FLOW_THRESHOLD = 0.7f;   // > 70% of speed limit
    constexpr float SLOW_THRESHOLD = 0.3f;        // 30-70% of speed limit
    constexpr float CONGESTION_THRESHOLD = 0.3f;  // < 30% of speed limit

    constexpr float CONGESTION_PERCENTAGE_THRESHOLD = 0.5f;
}

// Prediction System Constants
namespace PredictionConfig {
    constexpr int MAX_HISTORY_SIZE = 120;         // 2 minutes at 1 sample/second
    constexpr float PREDICTION_INTERVAL = 5.0f;   // Update every 5 seconds
    constexpr float PREDICTION_ALPHA = 0.3f;      // Exponential smoothing alpha
    constexpr int MOVING_AVERAGE_WINDOW = 10;

    constexpr float PEAK_HOUR_5MIN_MULTIPLIER = 0.7f;  // 30% slower
    constexpr float PEAK_HOUR_10MIN_MULTIPLIER = 0.6f; // 40% slower

    constexpr float MIN_PREDICTED_SPEED = 5.0f;
    constexpr float MIN_CONFIDENCE_THRESHOLD = 0.6f;
}

// Color Configuration
namespace ColorConfig {
    constexpr unsigned char FREE_FLOW_R = 0;
    constexpr unsigned char FREE_FLOW_G = 200;
    constexpr unsigned char FREE_FLOW_B = 0;

    constexpr unsigned char SLOW_R = 255;
    constexpr unsigned char SLOW_G = 255;
    constexpr unsigned char SLOW_B = 0;

    constexpr unsigned char CONGESTED_R = 255;
    constexpr unsigned char CONGESTED_G = 50;
    constexpr unsigned char CONGESTED_B = 50;

    constexpr unsigned char BLOCKED_R = 100;
    constexpr unsigned char BLOCKED_G = 100;
    constexpr unsigned char BLOCKED_B = 100;

    constexpr unsigned char PREDICTED_CONGESTION_R = 128;
    constexpr unsigned char PREDICTED_CONGESTION_G = 0;
    constexpr unsigned char PREDICTED_CONGESTION_B = 128;

    constexpr unsigned char BACKGROUND_R = 25;
    constexpr unsigned char BACKGROUND_G = 25;
    constexpr unsigned char BACKGROUND_B = 35;

    constexpr unsigned char PANEL_R = 40;
    constexpr unsigned char PANEL_G = 40;
    constexpr unsigned char PANEL_B = 50;
}

// Font Paths
namespace FontConfig {
    const char* const FONT_PATHS[] = {
        "arial.ttf",
        "fonts/arial.ttf",
        "../fonts/arial.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/tahoma.ttf",
        "C:/Windows/Fonts/calibri.ttf"
    };
    constexpr int FONT_PATH_COUNT = 6;
}

// File Paths
namespace FileConfig {
    constexpr const char* WARNING_TEXTURE_PATH = "warning.png";
    constexpr int WARNING_TEXTURE_SIZE = 32;
}

// Input Constants
namespace InputConfig {
    constexpr sf::Uint32 BACKSPACE_KEY = 8;
    constexpr sf::Uint32 ENTER_KEY = 13;
    constexpr sf::Uint32 DIGIT_START = 48;
    constexpr sf::Uint32 DIGIT_END = 57;
}
