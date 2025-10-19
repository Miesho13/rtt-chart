#ifndef CSV_H
#define CSV_H

#include <vector>
#include <array>
#include <string>
#include <time.h>
#include <cctype>
#include <print>
#include <mutex>
#include <fstream>
#include <regex>

using std::vector;
using std::array;
using std::string;
using std::isdigit;
using std::println;

namespace csv {
    struct probe {
        std::vector<double> value;
        time_t timestep;
    };

    template<int max_buffered_probes> class template_csv {
    public:


        template_csv(char delimiter) : _delimiter(delimiter) {
            probes.reserve(max_buffered_probes);
            _reset_value = max_buffered_probes;
            _maintain_value = 50;
        }

        static bool is_csv_line(const std::string& line) {
            static const std::regex pattern(R"(^I\s*\([^)]*\)\s*csv:)");
            return std::regex_search(line, pattern);
        }

        static std::string strip_csv_prefix(const std::string& line) {
            static const std::regex pattern(R"(^I\s*\([^)]*\)\s*csv:\s*)");
            return std::regex_replace(line, pattern, "");
        }


        void push(const string csv_line) {
            if (probes.size() == _reset_value) {
                reset();
            }

            string val = "";
            probe new_probe;
            new_probe.value.reserve(10);

            for (auto c : csv_line) {
                if (isdigit(c)) {
                    val += c;
                }
                else if (c == '.') {
                    val += c;
                }
                else if (c == _delimiter) {
                    if (val.size() == 0) {
                        val = "";
                        continue;
                    }

                    double val_f = std::stod(val);
                    new_probe.value.push_back(val_f);

                    val = "";
                }
            }

            if (new_probe.value.size() == 0) {
                return;
            }

            new_probe.timestep = time(NULL);
            probes.push_back(new_probe);
        }

        void print_csv() {
            for (auto prob : probes) {
                println("val:");
                for (auto val : prob.value) {
                    std::println("  {} ", val);
                }
                println("timestamp {}", prob.timestep);
                println("------------------------");
            }
        }

        std::vector<probe>* get() {
            return &probes;
        }

        void lock() {
            mut.lock();
        }

        void unlock() {
            mut.unlock();
        }

        void set_reset_value(int val) {
            _reset_value = val;
        }

        int get_bufferd_probe_count() const {
            return _reset_value; 
        }

        void set_reset(int val) {
            _reset_value = val;
        }

        void set_maintain(int val) {
            _maintain_value = val;
        }

        void reset() {
            vector tmp = probes;
            probes.clear();
            probes.insert(probes.begin(), tmp.end() - _maintain_value, tmp.end());
        }

        void save_csv(std::string name) {
            mut.lock();

            std::ofstream file;
            if (name.size() == 0) {
                auto time_name = time(NULL);
                std::string file_name = std::format("./{}_output.csv", time_name);
                file.open(file_name);
            } 
            else {
                file.open(name);
            }

            if (!file.is_open()) {
                println("File cant be crated");
                return;
            }

            std::string header = "No.; time; ";

            for (int i = 0; i < probes[0].value.size(); i++) {
                header += std::format("chart {}; ", i + 1);
            }

            file << header << '\n';

            for (int row = 0; row < probes.size(); row++) {
                std::string line = std::format("{}; ", row);
                line += std::format("{}; ", probes[row].timestep);
                for (auto val : probes[row].value) {
                    line += std::format("{}; ", val);
                }

                file << line << '\n';
            }

            mut.unlock();
        }

    private:
        std::mutex mut;

        std::vector<probe> probes;
        int ptr;

        char _delimiter;

        int _reset_value;
        int _maintain_value;
    };


    // GLOBEL INSTANCE FOR APLICATION CSV BUFFER
    template_csv<10*1024>* inst() {
        static template_csv<10*1024> _csv(';');
        return &_csv;
    }
}


#endif // CSV_H