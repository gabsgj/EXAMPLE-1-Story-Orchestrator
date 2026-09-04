# EXAMPLE 1: Autonomous Story Orchestrator

**Vector Embedding, Capability Composition, and Narrative Synthesis**  
*Qualitative & Narrative World Space*

---

## 1. Overview and Motivation

This example provides an autonomous, modular C++ implementation of the **Formal Capability Embedding & Composition Model**.

In narrative orchestration, the application domain is a dynamic story world. The central objective is to reach a story resolution satisfying all narrative goal conditions without violating safety constraints (such as violence or non-kid-friendly content).

> **CRITICAL CONCEPT: CAPABILITIES ARE TRANSITIONS (NOT STATES)**
> - A **State** ($S$) describes the condition of the story world at a point in time (e.g., characters introduced, rabbit asleep, finish line reached).
> - A **Capability** ($C_i$) is a **Transition / Operation / Arrow** ($S \xrightarrow{C_i} S'$) that transforms state variables via effects ($E_i$) when its preconditions ($P_i$) are satisfied.

---

## 2. Mathematical Formulation & Domain Mapping

### 2.1 Formal Application Model (Section 3)
$$\mathcal{A} = (\mathcal{S}, \mathcal{C}, S_I, G, \mathcal{R}, \mathcal{K})$$
- $\mathcal{S}$: State space of narrative variables.
- $\mathcal{C}$: Set of available narrative beat capabilities (transitions).
- $S_I$: Initial state (e.g., race not started, characters not introduced).
- $G$: Goal specification (e.g., race completed, moral learned, story finished).
- $\mathcal{R}$: Set of required resources (e.g., characters, props, landmarks).
- $\mathcal{K}$: Global safety constraints and hazard barriers (e.g., kid-friendly, no violence).

### 2.2 Formal Capability Representation (Section 4)
Each capability is represented as an 11-tuple:
$$C_i = (T_i, I_i, O_i, P_i, E_i, K_i, R_i, Q_i, \operatorname{Rel}_i, A_i, M_i)$$
- $T_i$: Capability type (`NARRATIVE_BEAT`, `COMPOSITE_BEAT`).
- $I_i, O_i$: Inputs and outputs.
- $P_i$: Preconditions describing when the beat is causally applicable ($S \models P_i$).
- $E_i$: Effects describing the state transition ($S' = \operatorname{Apply}(S, E_i)$).
- $K_i$: Capability-specific safety constraints.
- $R_i$: Resource requirements.
- $Q_i$: Operational quality attributes ($C_{\text{time}}, C_{\text{money}}, C_{\text{resource}}, C_{\text{risk}}$).
- $\operatorname{Rel}_i \in [0, 1]$: Reliability score.
- $A_i \in \{0, 1\}$: Availability.
- $M_i$: Execution mechanism (`DIRECT_SYNTHESIS`).

### 2.3 Vector Embedding (Section 6)
- $\phi_S: \mathcal{S} \to \mathbb{R}^{d_s}$: Encodes world state variables.
- $\phi_G: G \to \mathbb{R}^{d_g}$: Encodes target goal conditions.
- $\phi_C: \mathcal{C} \to \mathbb{R}^{d_c}$: Encodes the full capability transition vector:
  $$\mathbf{v}(C_i) = [\mathbf{v}_{\text{pre}}, \mathbf{v}_{\text{eff}}, \mathbf{v}_{\text{ops}}]$$

### 2.4 Precondition-Effect Compatibility (Section 6.1 Property 3)
Measures whether capability $C_1$ causally enables capability $C_2$:
$$\operatorname{Comp}(C_1, C_2) = \frac{\mathbf{v}_{\text{eff}}(C_1) \cdot \mathbf{v}_{\text{pre}}(C_2)}{\|\mathbf{v}_{\text{eff}}(C_1)\| \|\mathbf{v}_{\text{pre}}(C_2)\| + \epsilon}$$

### 2.5 Capability Composition (Section 5)
Given $C_1: S_0 \to S_1$ and $C_2: S_1 \to S_2$, the composite capability is:
$$C_{12} = C_2 \circ C_1: S_0 \to S_2$$
- Preconditions: $P(C_{12}) = P(C_1) \cup (P(C_2) \setminus E(C_1))$
- Effects: $E(C_{12}) = E(C_1) \oplus E(C_2)$ (overwritten by $C_2$)
- Operational QoS: Time cost is additive, reliability is multiplicative.

---

## 3. Modular Code Architecture

```
EXAMPLE 1 - Story Orchestrator/
├── include/
│   ├── types.hpp            <- Formal State, Goal, and Capability 11-tuple model
│   ├── embedding.hpp        <- Vector embedding engine, compatibility, & composition
│   ├── planner.hpp          <- Goal-directed capability transition planner
│   └── narrative_engine.hpp <- JSON parser, template rendering, & artifact export
├── src/
│   └── main.cpp             <- CLI interface, vector display, & execution pipeline
├── third_party/
│   └── nlohmann/json.hpp    <- Vendored single-header JSON library
├── rabbit_tortoise_story.json <- Aesop's Rabbit and Tortoise benchmark
├── the_lion_and_the_mouse.json<- Aesop's Lion and Mouse benchmark
├── space_rescue_mission.json  <- Sci-fi cooperative Mars rescue benchmark
├── teamwork_adventure.json    <- Cooperative obstacle puzzle benchmark
├── Makefile                   <- Self-contained C++17 build script
├── README.md                  <- This documentation
├── USAGE.md                   <- Step-by-step usage guide and commands
├── DELIVERABLE_1_FORMAL_EMBEDDING_DESIGN.md
├── DELIVERABLE_2_IMPLEMENTATION.md
├── DELIVERABLE_3_EXPERIMENTAL_DATASET.md
└── DELIVERABLE_4_TECHNICAL_REPORT.md
```

---

## 4. Compilation and Quick Start

```bash
# Compile
make

# Run the complete pipeline
./story_orchestrator rabbit_tortoise_story.json

# Run with vector embeddings, compatibility matrix, and composition
./story_orchestrator rabbit_tortoise_story.json --vectors --compatibility --compose
```
