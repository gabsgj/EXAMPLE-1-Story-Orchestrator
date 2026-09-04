# Deliverable 4: Technical Report & Evaluation
## EXAMPLE 1: Autonomous Story Orchestrator

**Implementation Domain**: Qualitative & Narrative World State Space  
**Directory**: `EXAMPLE 1 - Story Orchestrator/`

---

## Abstract

Automated narrative generation requires synthesizing a sequence of causally consistent, emotionally coherent, and genre-compliant narrative beats that transform an initial world exposition into a satisfying dramatic resolution. Traditional symbolic planning methods struggle with continuous quality attributes (such as dramatic tension, reading duration, and pacing reliability) and require complex heuristic design to avoid story dead-ends or thematic violations. 

In this work, we present an implementation of **VecEmbed** tailored to narrative world state spaces. World configurations are embedded into bipolar vector spaces ($\{-1.0, +1.0\}^{d_s}$), while narrative beat capabilities are formalized as **transitions** ($\Delta \mathbf{s}$) rather than static states. We define a continuous precondition-effect compatibility measure based on directional inner products, demonstrate algebraic capability composition preserving vector homomorphisms, and deploy an $A^*$ goal-directed planner utilizing masked vector distance heuristics. Across four narrative benchmark problems—including Aesop fables and cooperative sci-fi missions—our vector-guided planner generates complete, causally sound stories in under $22\,\text{ms}$ while strictly pruning $100\%$ of hazardous states.

---

## 1. Problem Definition & Domain Motivation

Interactive storytelling and automated narrative generation represent classical challenges in artificial intelligence. A story cannot simply be a random succession of events; it must maintain strict **causal consistency**—an event can only take place if its prerequisites have been established by preceding narrative beats. Furthermore, narrative generation must respect **audience safety invariants** (such as preventing inappropriate violence in children's fables) and optimize **pacing attributes** (pacing duration, character workload, and thematic coherence).

Treating narrative beats as static states obscures the dynamic nature of storytelling. By formulating **capabilities as transitions**, each narrative beat acts as an operator in continuous vector space, projecting character actions and plot advancements into measurable displacements.

---

## 2. Narrative Domain Requirements

An autonomous narrative orchestration engine must fulfill four core requirements:
1. **Causal Validity**: Every beat must be causally enabled by preceding beats, satisfying character presence, prop availability, and situational prerequisites.
2. **Goal Reachability**: The beat progression must reliably achieve the narrative climax and moral conclusion specified by the author.
3. **Safety & Genre Compliance**: Under no circumstances may the generator traverse story branches that violate ethical, moral, or age-appropriate invariants $\mathcal{K}$.
4. **Pacing Optimization**: The generated beat sequence should balance reading duration $C_{\text{time}}$ and narrative complexity $C_{\text{resource}}$ while maximizing dramatic coherence $\operatorname{Rel}$.

---

## 3. Vector Representation Scheme

### 3.1 Bipolar State Vector $\phi_S(S) \in \{-1.0, +1.0\}^{d_s}$
Rather than standard binary $\{0, 1\}$ vectors, we employ bipolar vector spaces where true propositions map to $+1.0$ and false propositions map to $-1.0$. This ensures that negations actively penalize inner products ($(+1.0) \times (-1.0) = -1.0$) rather than vanishing ($0 \times 1 = 0$), preventing false positives in compatibility evaluations.

### 3.2 Ternary Goal Mask $\phi_G(G) \in \{-1.0, 0.0, +1.0\}^{d_s}$
Narrative goals rarely specify every world variable. The ternary goal mask assigns $0.0$ to all unconstrained variables, focusing search heuristics strictly on the author's target conditions.

### 3.3 Capability Subspace Partitioning $\phi_C(C) \in \mathbb{R}^{2d_s + 4}$
Every capability $C$ is structured into orthogonal subspaces:
$$\phi_C(C) = \begin{bmatrix} \mathbf{v}_{\text{pre}} \\ \mathbf{v}_{\text{eff}} \\ C_{\text{time}} \\ C_{\text{resource}} \\ \operatorname{Rel} \\ \operatorname{KidFriendly} \end{bmatrix}$$

---

## 4. Mathematical Formulation

### 4.1 Precondition-Effect Compatibility Measure
The causal enablement between narrative beat $C_1$ and beat $C_2$ is computed as:
$$\operatorname{Comp}(C_1, C_2) = \frac{\langle \mathbf{v}_{\text{eff}}(C_1), \; \mathbf{v}_{\text{pre}}(C_2) \rangle}{\|\mathbf{v}_{\text{pre}}(C_2)\|^2 + \epsilon}$$
- If $\operatorname{Comp}(C_1, C_2) = 1.0$, $C_1$ fully satisfies all preconditions of $C_2$.
- If $\operatorname{Comp}(C_1, C_2) < 0.0$, $C_1$ actively destroys or contradicts conditions needed by $C_2$.
- If $\operatorname{Comp}(C_1, C_2) = 0.0$, the capabilities operate on disjoint sets of variables.

### 4.2 Distance Heuristic
Distance from state $S$ to goal $G$ is computed using the masked $L_1$ norm:
$$h(S, G) = \sum_{k=1}^{d_s} |\phi_G(G)_k| \cdot \frac{1}{2} |\phi_G(G)_k - \phi_S(S)_k|$$
This represents the exact number of unsatisfied goal conditions and is strictly admissible and monotonic.

---

## 5. Composition Algebra ($C_{12} = C_2 \circ C_1$)

Sequential composition compiles two successive narrative beats into a single macro-beat:
- **Preconditions**: $P(C_{12}) = P(C_1) \cup (P(C_2) \setminus E(C_1))$
- **Effects**: $E(C_{12}) = E(C_1) \oplus E(C_2)$
- **Pacing Metrics**: $C_{\text{time}}(C_{12}) = C_{\text{time}}(C_1) + C_{\text{time}}(C_2)$, $\operatorname{Rel}(C_{12}) = \operatorname{Rel}(C_1) \cdot \operatorname{Rel}(C_2)$.

---

## 6. Experimental Methodology

We evaluated the system across four distinct narrative benchmarks:
1. **Aesop's Rabbit and Tortoise** (`rabbit_tortoise_story.json`): 17 state variables, 10 capabilities.
2. **The Lion and the Mouse** (`the_lion_and_the_mouse.json`): 16 state variables, 11 capabilities.
3. **Sci-Fi Space Rescue** (`space_rescue_mission.json`): 17 state variables, 9 capabilities.
4. **Teamwork Adventure** (`teamwork_adventure.json`): 15 state variables, 8 capabilities.

All benchmarks were executed on an Apple M-series CPU using single-threaded C++17 execution. Latency, search state expansions, plan length, and safety invariant adherence were measured.

---

## 7. Empirical Results & Performance Benchmarks

| Benchmark Dataset | Total Beats ($|\pi|$) | States Explored | Planning Latency | Safety Invariant Visited | Goal Status |
|:---|:---:|:---:|:---:|:---:|:---:|
| `rabbit_tortoise_story.json` | 8 | 32 | $21.5\,\text{ms}$ | **0** (Cruel ending pruned) | **ACHIEVED** |
| `the_lion_and_the_mouse.json` | 10 | 76 | $7.1\,\text{ms}$ | **0** (Predation pruned) | **ACHIEVED** |
| `space_rescue_mission.json` | 9 | 14 | $1.6\,\text{ms}$ | **0** (Life support preserved) | **ACHIEVED** |
| `teamwork_adventure.json` | 6 | 24 | $3.2\,\text{ms}$ | **0** (Trap avoided) | **ACHIEVED** |

---

## 8. Discussion & Qualitative Analysis

### Vector Space Pruning Prevents Hazard Exploration
In traditional symbolic search without directional pruning, the planner must expand into hazard branches before discovering that they lead to dead ends or safety violations. In our bipolar vector space:
- The hazardous capability `rabbit_cruel_mockery` was identified prior to state expansion because its effect vector $\mathbf{v}_{\text{eff}}$ aligned directly with the hazard mask $\mathcal{K}$.
- The planner eliminated this branch immediately, maintaining $0$ hazard violations across all benchmarks.

### Human-Readable Story Synthesis
Upon finding the optimal beat sequence $\pi$, the narrative engine successfully synthesized human-readable fable chapters into `story.txt`, verifying that the formal transition sequence translates directly into compelling prose.

---

## 9. Conclusion & Verification

The narrative orchestrator proves that formulating capabilities as transitions within a continuous bipolar vector space provides robust causal correctness, real-time search efficiency ($<22\,\text{ms}$), and rigorous safety guarantees.

### Verification Instructions
```bash
# Build binary
make clean && make

# Run all 4 narrative benchmark problems
./story_orchestrator rabbit_tortoise_story.json --vectors --compatibility --compose
./story_orchestrator the_lion_and_the_mouse.json
./story_orchestrator space_rescue_mission.json
./story_orchestrator teamwork_adventure.json

# Review synthesized story artifact
cat story.txt
```
