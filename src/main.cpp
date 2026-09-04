#include "include/types.hpp"
#include "include/embedding.hpp"
#include "include/planner.hpp"
#include "include/narrative_engine.hpp"
#include <iostream>
#include <iomanip>

using namespace story;

void printHeader() {
    std::cout << "======================================================================\n";
    std::cout << "  EXAMPLE 1: AUTONOMOUS STORY ORCHESTRATOR                           \n";
    std::cout << "  Formal Capability Embeddings, Compatibility, & Narrative Synthesis \n";
    std::cout << "======================================================================\n\n";
}

void printUsage(const char* prog) {
    std::cout << "Usage:\n";
    std::cout << "  " << prog << " <problem_spec.json> [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --vectors         Display vector embeddings for states, goals, & capabilities\n";
    std::cout << "  --compatibility   Display precondition-effect compatibility matrix\n";
    std::cout << "  --compose         Demonstrate formal capability composition (C_2 o C_1)\n";
    std::cout << "  --help            Show this help message\n\n";
}

int main(int argc, char* argv[]) {
    printHeader();

    if (argc < 2) {
        printUsage(argv[0]);
        std::cout << "Defaulting to: rabbit_tortoise_story.json\n\n";
    }

    std::string jsonPath = (argc >= 2 && argv[1][0] != '-') ? argv[1] : "rabbit_tortoise_story.json";

    bool showVectors = false;
    bool showCompat = false;
    bool showCompose = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--vectors") showVectors = true;
        if (arg == "--compatibility") showCompat = true;
        if (arg == "--compose") showCompose = true;
        if (arg == "--help") {
            printUsage(argv[0]);
            return 0;
        }
    }

    try {
        std::cout << "Loading formal application specification: " << jsonPath << " ...\n";
        ApplicationProblem app = NarrativeEngine::loadFromJson(jsonPath);

        std::cout << "  -> Title:           " << app.title << "\n";
        std::cout << "  -> Author:          " << app.author << "\n";
        std::cout << "  -> Target Audience: " << app.targetAudience << "\n";
        std::cout << "  -> Initial Vars:    " << app.initialState.vars.size() << " variables in S_I\n";
        std::cout << "  -> Goal Conditions: " << app.goal.conditions.size() << " predicates in G\n";
        std::cout << "  -> Capabilities:    " << app.capabilities.size() << " available transitions in C\n";
        std::cout << "  -> Global Hazards:  " << app.globalConstraints.size() << " barrier invariants in K\n\n";

        // Initialize Vector Embedding Engine
        VectorEmbeddingEngine embedding;
        embedding.initialize(app);
        size_t d_s = embedding.stateDimension();
        size_t d_c = d_s * 2 + 4; // Preconditions + Effects + Operational attributes

        std::cout << "[EMBEDDING SPACE DEFINITION]\n";
        std::cout << "  State Space Dimension   d_s = " << d_s << "\n";
        std::cout << "  Goal Space Dimension    d_g = " << d_s << "\n";
        std::cout << "  Capability Dimension    d_c = " << d_c << " [P in R^" << d_s << ", E in R^" << d_s << ", Q/Rel/Safety in R^4]\n\n";

        // Display Vector Embeddings if requested
        if (showVectors) {
            std::cout << "----------------------------------------------------------------------\n";
            std::cout << "1. FORMAL VECTOR REPRESENTATIONS (phi_S, phi_G, phi_C)\n";
            std::cout << "----------------------------------------------------------------------\n";

            auto s0Vec = embedding.encodeState(app.initialState);
            std::cout << "Initial State Vector phi_S(S_I) in R^" << d_s << ":\n  [";
            for (size_t i = 0; i < s0Vec.size(); ++i) {
                std::cout << std::fixed << std::setprecision(1) << s0Vec[i] << (i + 1 < s0Vec.size() ? ", " : "");
            }
            std::cout << "]\n\n";

            auto gVec = embedding.encodeGoal(app.goal);
            std::cout << "Goal Target Vector phi_G(G) in R^" << d_s << ":\n  [";
            for (size_t i = 0; i < gVec.size(); ++i) {
                std::cout << std::fixed << std::setprecision(1) << gVec[i] << (i + 1 < gVec.size() ? ", " : "");
            }
            std::cout << "]\n\n";

            std::cout << "Sample Capability Vectors phi_C(C_i) in R^" << d_c << ":\n";
            for (size_t k = 0; k < std::min(app.capabilities.size(), size_t(3)); ++k) {
                const auto& c = app.capabilities[k];
                auto cVec = embedding.encodeCapability(c);
                std::cout << "  " << std::left << std::setw(28) << c.id << ": [";
                for (size_t i = 0; i < std::min(cVec.size(), size_t(8)); ++i) {
                    std::cout << std::fixed << std::setprecision(1) << cVec[i] << ", ";
                }
                std::cout << "... (dim=" << cVec.size() << ")]\n";
            }
            std::cout << "\n";
        }

        // Display Compatibility Matrix if requested
        if (showCompat) {
            std::cout << "----------------------------------------------------------------------\n";
            std::cout << "2. PRECONDITION-EFFECT COMPATIBILITY MATRIX (Section 6.1 Property 3)\n";
            std::cout << "   Comp(C_i, C_j) = (E_i . P_j) / (||E_i|| * ||P_j|| + eps)\n";
            std::cout << "----------------------------------------------------------------------\n";
            size_t nShow = std::min(app.capabilities.size(), size_t(5));
            std::cout << std::left << std::setw(20) << "From \\ To";
            for (size_t j = 0; j < nShow; ++j) {
                std::cout << std::setw(12) << app.capabilities[j].id.substr(0, 10);
            }
            std::cout << "\n";
            for (size_t i = 0; i < nShow; ++i) {
                std::cout << std::left << std::setw(20) << app.capabilities[i].id.substr(0, 18);
                for (size_t j = 0; j < nShow; ++j) {
                    double comp = embedding.calculateCompatibility(app.capabilities[i], app.capabilities[j]);
                    std::cout << std::fixed << std::setprecision(2) << std::setw(12) << comp;
                }
                std::cout << "\n";
            }
            std::cout << "\n";
        }

        // Display Capability Composition if requested
        if (showCompose && app.capabilities.size() >= 2) {
            std::cout << "----------------------------------------------------------------------\n";
            std::cout << "3. CAPABILITY COMPOSITION: C_12 = C_2 o C_1 (Section 5)\n";
            std::cout << "----------------------------------------------------------------------\n";
            const auto& c1 = app.capabilities[0];
            const auto& c2 = app.capabilities[1];
            Capability c12 = embedding.composeCapabilities(c1, c2);

            std::cout << "Component C_1: " << c1.id << " (" << c1.title << ")\n";
            std::cout << "Component C_2: " << c2.id << " (" << c2.title << ")\n";
            std::cout << "Composite C_12: " << c12.id << "\n";
            std::cout << "  -> Title:           " << c12.title << "\n";
            std::cout << "  -> Type:            " << c12.type << "\n";
            std::cout << "  -> Preconditions:   " << c12.preconditions.size() << " combined\n";
            std::cout << "  -> Effects:         " << c12.effects.size() << " combined/overwritten\n";
            std::cout << "  -> Composed Cost:   " << c12.qos.timeCost << " (additive time)\n";
            std::cout << "  -> Composed Rel:    " << c12.reliability << " (multiplicative reliability)\n\n";
        }

        // Planning: Synthesizing the complete trajectory
        std::cout << "----------------------------------------------------------------------\n";
        std::cout << "PLANNING & EXECUTION: SEARCHING FOR GOAL-SATISFYING TRANSITIONS\n";
        std::cout << "----------------------------------------------------------------------\n";
        CapabilityPlanner planner(app, embedding);
        PlanResult plan = planner.findPlan();

        if (!plan.success) {
            std::cerr << "Error: No kid-friendly narrative trajectory satisfied the goal.\n";
            return 1;
        }

        std::cout << "Plan Synthesis Status: SUCCESS [OPTIMAL NARRATIVE REACHED]\n";
        std::cout << "Planning Latency:      " << plan.planningLatencyMicroseconds << " microseconds\n";
        std::cout << "Nodes Expanded:        " << plan.nodesExpanded << " states evaluated\n";
        std::cout << "Plan Sequence Length:  " << plan.plannedCapabilities.size() << " capability transitions\n\n";

        std::cout << "Ordered Sequence of Transitions (Capabilities as Transitions):\n";
        for (size_t i = 0; i < plan.plannedCapabilities.size(); ++i) {
            const auto& cap = plan.plannedCapabilities[i];
            std::cout << "  Step " << (i + 1) << ": [" << cap.type << "] " << cap.id << " -> " << cap.title << "\n";
        }
        std::cout << "\n";

        // Synthesize and Render the Story
        std::cout << "======================================================================\n";
        std::cout << "          " << app.title << "\n";
        std::cout << "          (A Kid-Friendly Moral Fable Synthesized by VecEmbed)\n";
        std::cout << "======================================================================\n\n";

        State curState = app.initialState;
        for (const auto& cap : plan.plannedCapabilities) {
            curState = cap.apply(curState);
            std::string paragraph = NarrativeEngine::renderTemplate(cap.textTemplate, curState);
            if (!paragraph.empty()) {
                std::cout << paragraph << "\n\n";
            }
        }

        std::cout << "======================================================================\n";
        std::cout << "THE END\n";
        std::cout << "======================================================================\n\n";

        // Export Artifacts
        NarrativeEngine::exportStoryText("story.txt", app, plan);

        std::cout << "Artifacts Successfully Exported:\n";
        std::cout << "  - Story Text:         story.txt\n\n";

    } catch (const std::exception& ex) {
        std::cerr << "Fatal Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
