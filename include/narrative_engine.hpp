#pragma once

#include "types.hpp"
#include "embedding.hpp"
#include "planner.hpp"
#include <fstream>
#include <regex>

namespace story {

class NarrativeEngine {
public:
    static ApplicationProblem loadFromJson(const std::string& filePath) {
        std::ifstream f(filePath);
        if (!f.is_open()) {
            throw std::runtime_error("Cannot open problem specification: " + filePath);
        }
        json j;
        f >> j;

        ApplicationProblem app;
        app.title = j.value("storyTitle", "Untitled Story");
        app.author = j.value("author", "VecEmbed Orchestrator");
        app.targetAudience = j.value("targetAudience", "General");

        // Load initial state variables S_I
        if (j.contains("initialState") && j["initialState"].is_object()) {
            for (auto& [k, v] : j["initialState"].items()) {
                if (v.is_boolean()) app.initialState.set(k, Value(v.get<bool>()));
                else if (v.is_number_integer()) app.initialState.set(k, Value(v.get<int64_t>()));
                else if (v.is_number_float()) app.initialState.set(k, Value(v.get<double>()));
                else if (v.is_string()) app.initialState.set(k, Value(v.get<std::string>()));
            }
        }

        // Load goal conditions G
        if (j.contains("goal") && j["goal"].is_array()) {
            for (const auto& gItem : j["goal"]) {
                Condition cond;
                cond.variable = gItem.value("variable", "");
                cond.op = gItem.value("op", "==");
                if (gItem.contains("expectedValue")) {
                    const auto& ev = gItem["expectedValue"];
                    if (ev.is_boolean()) cond.expectedValue = Value(ev.get<bool>());
                    else if (ev.is_number_integer()) cond.expectedValue = Value(ev.get<int64_t>());
                    else if (ev.is_number_float()) cond.expectedValue = Value(ev.get<double>());
                    else if (ev.is_string()) cond.expectedValue = Value(ev.get<std::string>());
                }
                app.goal.conditions.push_back(cond);
            }
        }

        // Load hazards / global constraints K
        if (j.contains("hazards") && j["hazards"].is_array()) {
            for (const auto& hItem : j["hazards"]) {
                Condition cond;
                cond.variable = hItem.value("variable", "");
                cond.op = hItem.value("op", "==");
                if (hItem.contains("expectedValue")) {
                    const auto& ev = hItem["expectedValue"];
                    if (ev.is_boolean()) cond.expectedValue = Value(ev.get<bool>());
                    else if (ev.is_number_integer()) cond.expectedValue = Value(ev.get<int64_t>());
                    else if (ev.is_number_float()) cond.expectedValue = Value(ev.get<double>());
                    else if (ev.is_string()) cond.expectedValue = Value(ev.get<std::string>());
                }
                app.globalConstraints.push_back(cond);
            }
        }

        // Load capabilities C
        if (j.contains("capabilities") && j["capabilities"].is_array()) {
            for (const auto& cItem : j["capabilities"]) {
                Capability cap;
                cap.id = cItem.value("id", "cap_" + std::to_string(app.capabilities.size()));
                cap.title = cItem.value("title", cap.id);
                cap.textTemplate = cItem.value("textTemplate", "");
                cap.type = cItem.value("type", "NARRATIVE_BEAT");
                cap.isKidFriendly = cItem.value("isKidFriendly", true);
                cap.violenceLevel = cItem.value("violenceLevel", 0.0);
                cap.reliability = cItem.value("reliability", 1.0);
                cap.availability = cItem.value("availability", 1.0);

                if (cItem.contains("qos")) {
                    cap.qos.timeCost = cItem["qos"].value("timeCost", 1.0);
                    cap.qos.moneyCost = cItem["qos"].value("moneyCost", 0.0);
                    cap.qos.resourceCost = cItem["qos"].value("resourceCost", 1.0);
                }

                if (cItem.contains("preconditions") && cItem["preconditions"].is_array()) {
                    for (const auto& pItem : cItem["preconditions"]) {
                        Condition cond;
                        cond.variable = pItem.value("variable", "");
                        cond.op = pItem.value("op", "==");
                        if (pItem.contains("expectedValue")) {
                            const auto& ev = pItem["expectedValue"];
                            if (ev.is_boolean()) cond.expectedValue = Value(ev.get<bool>());
                            else if (ev.is_number_integer()) cond.expectedValue = Value(ev.get<int64_t>());
                            else if (ev.is_number_float()) cond.expectedValue = Value(ev.get<double>());
                            else if (ev.is_string()) cond.expectedValue = Value(ev.get<std::string>());
                        }
                        cap.preconditions.push_back(cond);
                    }
                }

                if (cItem.contains("effects") && cItem["effects"].is_array()) {
                    for (const auto& eItem : cItem["effects"]) {
                        Effect eff;
                        eff.variable = eItem.value("variable", "");
                        eff.op = eItem.value("op", "SET");
                        if (eItem.contains("value")) {
                            const auto& val = eItem["value"];
                            if (val.is_boolean()) eff.value = Value(val.get<bool>());
                            else if (val.is_number_integer()) eff.value = Value(val.get<int64_t>());
                            else if (val.is_number_float()) eff.value = Value(val.get<double>());
                            else if (val.is_string()) eff.value = Value(val.get<std::string>());
                        }
                        cap.effects.push_back(eff);
                    }
                }

                app.capabilities.push_back(cap);
            }
        }

        return app;
    }

    static std::string renderTemplate(const std::string& templ, const State& s) {
        std::string result = templ;
        for (const auto& [k, v] : s.vars) {
            std::string placeholder = "{" + k + "}";
            size_t pos = 0;
            while ((pos = result.find(placeholder, pos)) != std::string::npos) {
                result.replace(pos, placeholder.length(), v.asString());
                pos += v.asString().length();
            }
        }
        return result;
    }

    static void exportStoryText(const std::string& filePath, const ApplicationProblem& app, const PlanResult& plan) {
        std::ofstream out(filePath);
        if (!out.is_open()) return;

        out << "======================================================================\n";
        out << "          " << app.title << "\n";
        out << "          (A Kid-Friendly Moral Fable Synthesized by VecEmbed)\n";
        out << "======================================================================\n\n";

        State curState = app.initialState;
        for (size_t i = 0; i < plan.plannedCapabilities.size(); ++i) {
            const auto& cap = plan.plannedCapabilities[i];
            curState = cap.apply(curState);
            std::string paragraph = renderTemplate(cap.textTemplate, curState);
            if (!paragraph.empty()) {
                out << paragraph << "\n\n";
            }
        }

        out << "======================================================================\n";
        out << "THE END\n";
        out << "======================================================================\n";
    }

    static void exportVisualizerManifest(const std::string& filePath, const ApplicationProblem& app, const PlanResult& plan) {
        json j;
        j["manifestVersion"] = "2.0";
        j["domain"] = "NarrativeSynthesis";
        j["title"] = app.title;
        j["success"] = plan.success;
        j["planningLatencyMicroseconds"] = plan.planningLatencyMicroseconds;

        // Path of capabilities executed
        std::vector<std::string> path;
        for (const auto& cap : plan.plannedCapabilities) {
            path.push_back(cap.id);
        }
        j["optimalPath"] = path;

        // Export all capabilities for visualization
        json capArr = json::array();
        for (const auto& c : app.capabilities) {
            json cObj;
            cObj["id"] = c.id;
            cObj["title"] = c.title;
            cObj["type"] = c.type;
            cObj["reliability"] = c.reliability;
            cObj["isKidFriendly"] = c.isKidFriendly;
            capArr.push_back(cObj);
        }
        j["capabilities"] = capArr;

        std::ofstream out(filePath);
        if (out.is_open()) {
            out << j.dump(2) << "\n";
        }
    }
};

} // namespace story
