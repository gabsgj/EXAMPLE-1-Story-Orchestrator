#pragma once

#include "types.hpp"
#include <cmath>
#include <numeric>
#include <iomanip>

namespace story {

// =============================================================================
// FORMAL VECTOR EMBEDDING ENGINE
// Section 6: Embedding Design Problem
// Deliverable 2: encode(state), encode(goal), encode(capability), compose(), similarity()
//
// BIPOLAR VECTOR SPACE FORMULATION:
// - Boolean state variables are embedded as {-1.0 (false), +1.0 (true)}
// - Preconditions are embedded as:
//     +1.0 : Requires true
//     -1.0 : Requires false
//      0.0 : Unconstrained / Don't care (orthogonal)
// - Effects are embedded as:
//     +1.0 : Sets true
//     -1.0 : Sets false
//      0.0 : Untouched / No change (orthogonal)
//
// This guarantees that inner products directly reflect algebraic compatibility:
//   E_1 . P_2 > 0  ==> Precondition satisfied
//   E_1 . P_2 < 0  ==> Precondition actively violated (conflict)
//   E_1 . P_2 == 0 ==> Precondition uninfluenced
// =============================================================================

class VectorEmbeddingEngine {
public:
    std::unordered_map<std::string, size_t> varIndex;
    std::vector<std::string> indexVar;

    size_t registerVariable(const std::string& varName) {
        auto it = varIndex.find(varName);
        if (it != varIndex.end()) return it->second;
        size_t idx = indexVar.size();
        varIndex[varName] = idx;
        indexVar.push_back(varName);
        return idx;
    }

    [[nodiscard]] size_t stateDimension() const {
        return indexVar.size();
    }

    void initialize(const ApplicationProblem& app) {
        for (const auto& kv : app.initialState.vars) registerVariable(kv.first);
        for (const auto& cond : app.goal.conditions) registerVariable(cond.variable);
        for (const auto& cap : app.capabilities) {
            for (const auto& pre : cap.preconditions) registerVariable(pre.variable);
            for (const auto& eff : cap.effects) registerVariable(eff.variable);
        }
    }

    // -------------------------------------------------------------------------
    // Deliverable 2: encode(state) -> phi_S(S) in R^{d_s}
    // Maps Boolean variables to {-1.0, +1.0} and numeric variables to their value
    // -------------------------------------------------------------------------
    [[nodiscard]] std::vector<double> encode(const State& s) const {
        std::vector<double> vec(stateDimension(), 0.0);
        for (size_t i = 0; i < stateDimension(); ++i) {
            const std::string& vname = indexVar[i];
            if (s.has(vname)) {
                Value val = s.getOr(vname, Value(false));
                if (std::holds_alternative<bool>(val.data)) {
                    vec[i] = std::get<bool>(val.data) ? 1.0 : -1.0;
                } else {
                    vec[i] = val.toNumeric();
                }
            } else {
                vec[i] = -1.0; // Default inactive / false state
            }
        }
        return vec;
    }

    [[nodiscard]] std::vector<double> encodeState(const State& s) const {
        return encode(s);
    }

    // -------------------------------------------------------------------------
    // Deliverable 2: encode(goal) -> phi_G(G) in R^{d_g}
    // Target conditions embedded with non-zero polarities (+1.0 / -1.0)
    // -------------------------------------------------------------------------
    [[nodiscard]] std::vector<double> encode(const Goal& g) const {
        std::vector<double> vec(stateDimension(), 0.0);
        for (const auto& cond : g.conditions) {
            auto it = varIndex.find(cond.variable);
            if (it != varIndex.end()) {
                if (std::holds_alternative<bool>(cond.expectedValue.data)) {
                    vec[it->second] = std::get<bool>(cond.expectedValue.data) ? 1.0 : -1.0;
                } else {
                    vec[it->second] = cond.expectedValue.toNumeric();
                }
            }
        }
        return vec;
    }

    [[nodiscard]] std::vector<double> encodeGoal(const Goal& g) const {
        return encode(g);
    }

    // Precondition Subspace Vector v_pre in R^{d_s}
    [[nodiscard]] std::vector<double> encodePreconditions(const Capability& c) const {
        std::vector<double> vec(stateDimension(), 0.0); // 0.0 = Don't care
        for (const auto& pre : c.preconditions) {
            auto it = varIndex.find(pre.variable);
            if (it != varIndex.end()) {
                if (std::holds_alternative<bool>(pre.expectedValue.data)) {
                    vec[it->second] = std::get<bool>(pre.expectedValue.data) ? 1.0 : -1.0;
                } else {
                    vec[it->second] = pre.expectedValue.toNumeric();
                }
            }
        }
        return vec;
    }

    // Effect Subspace Vector v_eff in R^{d_s}
    [[nodiscard]] std::vector<double> encodeEffects(const Capability& c) const {
        std::vector<double> vec(stateDimension(), 0.0); // 0.0 = Untouched
        for (const auto& eff : c.effects) {
            auto it = varIndex.find(eff.variable);
            if (it != varIndex.end()) {
                if (std::holds_alternative<bool>(eff.value.data)) {
                    vec[it->second] = std::get<bool>(eff.value.data) ? 1.0 : -1.0;
                } else {
                    vec[it->second] = eff.value.toNumeric();
                }
            }
        }
        return vec;
    }

    // -------------------------------------------------------------------------
    // Deliverable 2: encode(capability) -> phi_C(C) in R^{d_c}
    // Unified 11-tuple vector: [v_pre in R^{d_s}, v_eff in R^{d_s}, v_qos in R^4]
    // -------------------------------------------------------------------------
    [[nodiscard]] std::vector<double> encode(const Capability& c) const {
        std::vector<double> fullVec;
        auto pre = encodePreconditions(c);
        auto eff = encodeEffects(c);
        fullVec.insert(fullVec.end(), pre.begin(), pre.end());
        fullVec.insert(fullVec.end(), eff.begin(), eff.end());

        // Operational attributes (Q_i, Rel_i, Safety)
        fullVec.push_back(c.qos.timeCost);
        fullVec.push_back(c.qos.resourceCost);
        fullVec.push_back(c.reliability);
        fullVec.push_back(c.isKidFriendly ? 1.0 : 0.0);

        return fullVec;
    }

    [[nodiscard]] std::vector<double> encodeCapability(const Capability& c) const {
        return encode(c);
    }

    // -------------------------------------------------------------------------
    // Deliverable 2: similarity(x, y) -> Cosine similarity in R^d
    // -------------------------------------------------------------------------
    static double similarity(const std::vector<double>& v1, const std::vector<double>& v2) {
        if (v1.size() != v2.size() || v1.empty()) return 0.0;
        double dot = 0.0, n1 = 0.0, n2 = 0.0;
        for (size_t i = 0; i < v1.size(); ++i) {
            dot += v1[i] * v2[i];
            n1 += v1[i] * v1[i];
            n2 += v2[i] * v2[i];
        }
        if (n1 < 1e-9 || n2 < 1e-9) return 0.0;
        return dot / (std::sqrt(n1) * std::sqrt(n2));
    }

    static double cosineSimilarity(const std::vector<double>& v1, const std::vector<double>& v2) {
        return similarity(v1, v2);
    }

    // -------------------------------------------------------------------------
    // Precondition-Effect Compatibility: Does C_1 enable C_2? (E_1 => P_2)
    // Section 6.1 Property 3: Inner product between v_eff(C1) and v_pre(C2)
    // -------------------------------------------------------------------------
    [[nodiscard]] double calculateCompatibility(const Capability& c1, const Capability& c2) const {
        auto eff1 = encodeEffects(c1);
        auto pre2 = encodePreconditions(c2);

        double dot = 0.0;
        double reqNormSq = 0.0;

        for (size_t i = 0; i < stateDimension(); ++i) {
            if (pre2[i] != 0.0) {
                reqNormSq += pre2[i] * pre2[i];
                dot += eff1[i] * pre2[i];
            }
        }

        if (reqNormSq < 1e-9) return 1.0; // C2 has no preconditions, trivially compatible
        return dot / reqNormSq;
    }

    // Goal Relevance: Cosine alignment between capability effects and goal target
    [[nodiscard]] double goalRelevance(const Capability& c, const Goal& g) const {
        auto effVec = encodeEffects(c);
        auto gVec = encode(g);
        double dot = 0.0, normEff = 0.0, normG = 0.0;
        for (size_t i = 0; i < stateDimension(); ++i) {
            if (gVec[i] != 0.0) {
                normG += gVec[i] * gVec[i];
                if (effVec[i] != 0.0) {
                    dot += effVec[i] * gVec[i];
                    normEff += effVec[i] * effVec[i];
                }
            }
        }
        if (normEff < 1e-9 || normG < 1e-9) return 0.0;
        return dot / (std::sqrt(normEff) * std::sqrt(normG));
    }

    // -------------------------------------------------------------------------
    // Deliverable 2: compose(c1, c2) -> C_12 = C_2 o C_1 (Section 5)
    // -------------------------------------------------------------------------
    [[nodiscard]] Capability compose(const Capability& c1, const Capability& c2) const {
        Capability comp;
        comp.id = c1.id + "_O_" + c2.id;
        comp.title = "Composite: (" + c2.title + " o " + c1.title + ")";
        comp.type = "COMPOSITE_BEAT";

        // Preconditions: P(C_12) = P(C_1) U (P(C_2) \ E(C_1))
        comp.preconditions = c1.preconditions;
        for (const auto& p2 : c2.preconditions) {
            bool satisfied = false;
            for (const auto& e1 : c1.effects) {
                if (e1.variable == p2.variable && e1.value == p2.expectedValue) {
                    satisfied = true;
                    break;
                }
            }
            if (!satisfied) comp.preconditions.push_back(p2);
        }

        // Effects: E(C_12) = E(C_1) overridden by E(C_2)
        comp.effects = c1.effects;
        for (const auto& e2 : c2.effects) {
            bool overwritten = false;
            for (auto& ce : comp.effects) {
                if (ce.variable == e2.variable) {
                    ce = e2;
                    overwritten = true;
                    break;
                }
            }
            if (!overwritten) comp.effects.push_back(e2);
        }

        // Operational attributes
        comp.qos.timeCost = c1.qos.timeCost + c2.qos.timeCost;
        comp.qos.resourceCost = std::max(c1.qos.resourceCost, c2.qos.resourceCost);
        comp.qos.risk = 1.0 - ((1.0 - c1.qos.risk) * (1.0 - c2.qos.risk));
        comp.reliability = c1.reliability * c2.reliability;
        comp.isKidFriendly = c1.isKidFriendly && c2.isKidFriendly;

        return comp;
    }

    [[nodiscard]] Capability composeCapabilities(const Capability& c1, const Capability& c2) const {
        return compose(c1, c2);
    }
};

} // namespace story
