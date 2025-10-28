#ifndef APP_HPP 
#define APP_HPP 

#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER

#include <cstdint>
#include <format>
#include <functional>
#include <algorithm>

#include "csv.hpp"
#include "raylib.h" 


using std::vector;

namespace app {

    static class _window_state {
    public:
        uint32_t width;
        uint32_t heigth;
    } window_context;

    static class _plot_window {
    public:
        static constexpr Color plot_color[10] = {
            {0xef, 0x56, 0x58, 0xff}, // red
            {0x42, 0xa5, 0xf5, 0xff}, // Blue
            {0x66, 0xbb, 0x6a, 0xff}, // Green
            {0xff, 0xca, 0x28, 0xff}, // Orange/Yellow
            {0xab, 0x47, 0xbc, 0xff}, // Purple
            {0x26, 0xa6, 0x9a, 0xff}, // Teal/Cyjan
            {0xff, 0x52, 0x22, 0xff}, // Deep Orange
            {0x8b, 0xc3, 0x4a, 0xff}, // Light Green
            {0xe9, 0x1e, 0x63, 0xff}, // Pink
            {0x60, 0x7d, 0x8b, 0xff}, // Blue Grey
        };

        _plot_window() { 
            chose_scale = 0;

            scale.min_val = 0;
            scale.max_val = 500;
            scale.scale_step = 10;

            display_probe_max = 50;
            ui_plot_state.visible_probe = false;
            ui_plot_state.probe_size = 3;
            ui_plot_state.pause = false;

            command_mod_on = false;
            str_command = ": ";
        }

        void input_text_keyboard() {
            for (int key = KEY_A; key <= KEY_Z; key++) {
                if (IsKeyPressed(key)) {
                    char c = (char)('A' + (key - KEY_A));
                    str_command += c;
                }
            }

            for (int key = KEY_ZERO; key <= KEY_NINE; key++) {
                if (IsKeyPressed(key)) {
                    char c = (char)('0' + (key - KEY_ZERO));
                    str_command += c;
                }
            }

            if (IsKeyPressed(KEY_SPACE)) {
                str_command += ' ';
            }

            if (IsKeyPressed(KEY_BACKSPACE) && !str_command.empty()) {
                    str_command.pop_back();
            }

            if (IsKeyPressed(KEY_ENTER)) {
                // e.g. process the command, then clear:

                process_command(str_command);

                str_command.clear();
                str_command += ": ";
                command_mod_on = false;
            }
        }

        struct command_handle {
            string prefix;
            std::function<void(const std::string&)> call;
        };

        vector<command_handle> command_table = {
            {   
                ": SET YMAX",
                [this](const std::string& arg) {
                    try { 
                        int arg_val = std::stoi(arg); 
                        this->scale.max_val = arg_val;
                    }
                    catch (...) { 
                        return;
                    }
                }
            },
            {
                ": SET YMIN",
                [this](const std::string& arg) {
                    try { 
                        int arg_val = std::stoi(arg); 
                        this->scale.min_val = arg_val;
                    }
                    catch (...) { 
                        return;
                    }
                }
            },
            {
                ": SAVE",
                [this](const std::string& arg) {
                    csv::inst()->save_csv(arg);
                }
            },

        };

        void process_command(std::string command_str) {
            for (auto command : command_table) {
                if (command_str.rfind(command.prefix, 0) == 0) {
                    std::string arg = command_str.substr(command.prefix.size());
                    command.call(arg);
                    break;
                }
            }
        }

        void command() {
            if (!command_mod_on) {
                return;
            }
            int font_size = (int)(window_context.heigth*0.05);
            int padding = 5;
            DrawText(
                str_command.c_str(), 
                x_offset + padding,
                window_context.heigth - font_size - padding, 
                (int)(window_context.heigth*0.05), RAYWHITE
            );
        }

        void draw_scale() {
            DrawLineEx(
                {(float)x_offset, 0}, 
                {(float)x_offset, (float)window_context.heigth}, 
                1, RAYWHITE
            );

            float pixel_step = (float)window_context.heigth /  (float)scale.scale_step;
            double val_step  = ((double)scale.max_val - (double)scale.min_val) / (double)scale.scale_step;

            for (int div = 0; div < scale.scale_step; div++) {
                DrawLineEx(
                    {x_offset, div*pixel_step}, 
                    {(float)window_context.width, div*pixel_step}, 
                    1, {0x28, 0x28, 0x28, 0xff}
                );

                DrawLineEx(
                    {x_offset - 5, div*pixel_step}, 
                    {x_offset + 5, div*pixel_step}, 
                    1, RAYWHITE
                );

                auto val_str = std::format("{:.2f}", (double)scale.max_val - (double)val_step*div);
                DrawText(val_str.c_str(), 0, div*pixel_step, (int)(window_context.heigth*0.03), RAYWHITE);
            }
        }

        void draw_cursore() {
            for (int cursor_y = 0; cursor_y < 2; cursor_y++) {
                if (!cursors[cursor_y].enable) {
                    continue;
                }
                
                DrawLineEx( 
                    {x_offset, (float)cursors[cursor_y].y}, 
                    {(float)window_context.width, (float)cursors[cursor_y].y}, 
                    1, RAYWHITE 
                );

                auto val_str = std::format("{:.2f}", cursors[cursor_y].value);
                DrawText(val_str.c_str(), window_context.width - window_context.width*0.1, 
                         (float)cursors[cursor_y].y + 2, 18, RAYWHITE);
            }

            if (cursors[0].enable && cursors[1].enable) {

                auto val_str = std::format("{}", cursors[0].value - cursors[1].value);
                DrawText(val_str.c_str(), x_offset + 10, 
                         10, 20, RAYWHITE);
            }
        }

        // float probe_y_position(double val, int val_idx) {
        //     return (1.f - val/((double)scale.max_val - (double)scale.min_val)) * (double)window_context.heigth + scale.y_offset;
        // }

        float probe_y_position(double val, int val_idx) {
            double range = (double)scale.max_val - (double)scale.min_val;
            if (range == 0.0) range = 1.0;  // avoid divide by zero

            double normalized = (val - (double)scale.min_val) / range;
            normalized = std::clamp(normalized, 0.0, 1.0); // keep 0..1

            return (1.0f - (float)normalized) * (float)window_context.heigth + (float)scale.y_offset;
        }

        double probe_value_from_y(float y) {
            double range = (double)scale.max_val - (double)scale.min_val;
            if (range == 0.0) range = 1.0;  // avoid divide by zero

            double normalized = 1.0 - ((double)y - (double)scale.y_offset) / (double)window_context.heigth;
            normalized = std::clamp(normalized, 0.0, 1.0); // keep 0..1

            return (double)scale.min_val + normalized * range;
        }

        void draw_plots() {
            if (display_probe.size() < 2) {
                return;
            }

            for (int probe_idx = 0; probe_idx < display_probe.size() - 1; probe_idx++) {
                for (int val_idx = 0; val_idx < display_probe[probe_idx].value.size(); val_idx++) {
                    float y_old; 
                    float y_new; 

                    if (display_probe[probe_idx].value[val_idx] < scale.min_val) {
                        y_old  = window_context.heigth;
                    }
                    else if (display_probe[probe_idx].value[val_idx] > scale.max_val) {
                        y_old = 0;
                    }
                    else {
                        y_old 
                            = probe_y_position(display_probe[probe_idx].value[val_idx], val_idx);
                    }

                    if (display_probe[probe_idx + 1].value[val_idx] < scale.min_val) {
                        y_new  = window_context.heigth;
                    }
                    else if (display_probe[probe_idx + 1].value[val_idx] > scale.max_val) {
                        y_new  = 0;
                    }
                    else {
                        y_new 
                            = probe_y_position(display_probe[probe_idx + 1].value[val_idx], val_idx);
                    } 

                    float x_old 
                        = (double)probe_idx
                            *((double)window_context.width/(double)display_probe.size()) 
                            + x_offset;
                    float x_new 
                        = (probe_idx + 1)
                            *((double)window_context.width/(double)display_probe.size()) 
                            + x_offset;

                    
                    DrawLineEx( 
                        {x_old, y_old}, {x_new, y_new}, 
                        2, plot_color[val_idx % 10]
                    );

                    if (ui_plot_state.visible_probe) {
                        DrawCircleV(
                            {x_new, y_new}, 
                            ui_plot_state.probe_size, 
                            plot_color[val_idx % 10]
                        );
                    }
                }
            }
        }

        void populate_display_probe(vector<csv::probe> *probes) {
            if (ui_plot_state.pause) {
                return;
            }

            if (probes->size() == 0) {
                return;
            }

            display_probe.clear();
            if (probes->size() < display_probe_max) {
                display_probe.insert(display_probe.begin(), probes->begin(), probes->end());
                return;
            }

            display_probe.insert(
                display_probe.begin(), 
                probes->end() - display_probe_max, probes->end()
            );
        }

        void change_display_preobe(int val) {
            int new_display_probe = display_probe_max + val;
            if (new_display_probe <= 2) {
                return;
            }

            if (new_display_probe >= csv::inst()->get_bufferd_probe_count()) {
                return;
            }

            display_probe_max = new_display_probe;
        }

    public:
        vector<csv::probe> display_probe;
        int display_probe_max;
        float x_offset;

        int chose_scale;
        struct {
            int scale_step;
            int min_val;
            int max_val;
            int y_offset;
        } scale;

        struct {
            bool pause;
            int probe_size;
            bool visible_probe;
        } ui_plot_state;

        struct {
            bool enable;
            float y;
            float value;
        } cursors[2];

        bool command_mod_on;
        std::string str_command;

    } plot_window;

    static void update_window_size() {
        window_context.heigth = GetScreenHeight();
        window_context.width = GetScreenWidth();
        plot_window.x_offset = window_context.width * 0.10;
    }

    void input_event() {
        if (IsKeyPressed(KEY_SEMICOLON)) {
            plot_window.command_mod_on = true;
        }

        if (plot_window.command_mod_on) {
            plot_window.input_text_keyboard();
            return;
        }

        // show plot probes  
        if (IsKeyPressed(KEY_S)) {
            plot_window.ui_plot_state.visible_probe = !plot_window.ui_plot_state.visible_probe;
        }

        // change X axies 
        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            if (IsKeyDown(KEY_X)) {
                plot_window.change_display_preobe(-1);
            }
        } 
        else {
            if (IsKeyDown(KEY_X)) {
                plot_window.change_display_preobe(1);
            }
        } 

        if (IsKeyPressed(KEY_P)) {
            plot_window.ui_plot_state.pause = !plot_window.ui_plot_state.pause;
        }


        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {  
                plot_window.cursors[0].enable = false;
            }
            if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {  
                plot_window.cursors[1].enable = false;
            }
        }
        else {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {  
                plot_window.cursors[0].y = GetMouseY();
                plot_window.cursors[0].enable = true;
                plot_window.cursors[0].value = plot_window.probe_value_from_y(plot_window.cursors[0].y);
            }

            if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {  
                plot_window.cursors[1].y = GetMouseY();
                plot_window.cursors[1].enable = true;
                plot_window.cursors[1].value = plot_window.probe_value_from_y(plot_window.cursors[1].y);
            }
        }
    }

    void csv_proprety_update() {
        csv::inst()->set_maintain(plot_window.display_probe_max);
        csv::inst()->set_reset(10*1024);
    }

    void init() {
        SetConfigFlags(FLAG_WINDOW_RESIZABLE);

        InitWindow(800, 600, "char-plot");
        SetTargetFPS(144);
    }

    int run() {
        while (!WindowShouldClose()) { 
            input_event(); 
            update_window_size();
            csv_proprety_update();

            csv::inst()->lock();
            plot_window.populate_display_probe(csv::inst()->get());
            csv::inst()->unlock();

            BeginDrawing();

            ClearBackground((Color){0x0b, 0x0f, 0x16, 0xff});

            plot_window.draw_plots();
            plot_window.draw_scale();
            plot_window.draw_cursore();
            plot_window.command();

            EndDrawing();
        }

        return 0;
    }
}

#endif // APP_HPP 