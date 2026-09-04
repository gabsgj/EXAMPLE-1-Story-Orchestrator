#pragma once

#include "types.hpp"
#include "embedding.hpp"
#include <queue>
#include <chrono>

namespace story {

struct PlanNode {
    State state;
    std::vector<std::string> path; // Sequence of Capability IDs
    double gCost = 0.0;
    double hCost = 0.0;

    [[nodiscard]] double fCost() const { return gCost + hCost; }

    bool operator>(const PlanNode& other) const {
        return fCost() > other.fCost();
    }
};

struct PlanResult {
    bool success = false;
    std::vector<Capability> plannedCapabilities;
    State finalState;
    double totalCost = 0.0;
    double planningLatencyMicroseconds = 0.0;
    size_t nodesExpanded = 0;
};

class CapabilityPlanner {
private:
    const ApplicationProblem& app;
    const VectorEmbeddingEngine& embedding;

    [[nodiscard]] double heuristic(const State& s) const {
        auto sVec = embedding.encodeState(s);
        auto gVec = embedding.encodeGoal(app.goal);
        double dist = 0.0;
        for (size_t i = 0; i < sVec.size(); ++i) {
            if (gVec[i] != 0.0) {
                double diff = gVec[i] - sVec[i];
                dist += std::abs(diff);
            }
        }
        return dist;
    }

    [[nodiscard]] bool violatesHazards(const State& s) const {
        for (const auto& hazard : app.globalConstraints) {
            if (hazard.evaluate(s)) return true;
        }
        return false;
    }

public:
    CapabilityPlanner(const ApplicationProblem& problem, const VectorEmbeddingEngine& emb)
        : app(problem), embedding(emb) {}

    PlanResult findPlan() {
        auto startTime = std::chrono::high_resolution_clock::now();
        PlanResult result;

        // Check if initial state violates constraints
        if (violatesHazards(app.initialState)) {
            return result;
        }

        // Check if initial state already satisfies goal
        if (app.goal.isSatisfied(app.initialState)) {
            result.success = true;
            result.finalState = app.initialState;
            return result;
        }

        std::priority_queue<PlanNode, std::vector<PlanNode>, std::greater<PlanNode>> openSet;
        openSet.push({app.initialState, {}, 0.0, heuristic(app.initialState)});

        std::unordered_map<std::string, Capability> capMap;
        for (const auto& c : app.capabilities) {
            capMap[c.id] = c;
        }

        while (!openSet.empty()) {
            PlanNode cur = openSet.top();
            openSet.pop();
            result.nodesExpanded++;

            if (app.goal.isSatisfied(cur.state)) {
                auto endTime = std::chrono::high_resolution_clock::now();
                result.success = true;
                result.finalState = cur.state;
                result.totalCost = cur.gCost;
                result.planningLatencyMicroseconds = std::chrono::duration<double, std::micro>(endTime - startTime).count();

                for (const auto& cid : cur.path) {
                    result.plannedCapabilities.push_back(capMap[cid]);
                }
                return result;
            }

            // Expand valid transitions (Capabilities)
            for (const auto& cap : app.capabilities) {
                // Must be applicable to current state
                if (!cap.isApplicable(cur.state)) continue;

                // Apply transition: S' = Apply(S, E_i)
                State nextState = cap.apply(cur.state);

                // Prune if hazard violated
                if (violatesHazards(nextState)) continue;

                PlanNode nextNode;
                nextNode.state = nextState;
                nextNode.path = cur.path;
                nextNode.path.push_back(cap.id);
                nextNode.gCost = cur.gCost + cap.qos.totalCost();
                nextNode.hCost = heuristic(nextState);

                openSet.push(nextNode);
            }

            // Safety bound on depth
            if (result.nodesExpanded > 5000) break;
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        result.planningLatencyMicroseconds = std::chrono::duration<double, std::micro>(endTime - startTime).count();
        return result;
    }
};

} // namespace story
