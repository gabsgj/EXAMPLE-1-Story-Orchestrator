# Deliverable 2: Working Implementation
## EXAMPLE 1: Autonomous Story Orchestrator

**Implementation Domain**: Qualitative & Narrative World State Space  
**Directory**: `EXAMPLE 1 - Story Orchestrator/`

---

## 1. System Architecture and File Structure

The Story Orchestrator is implemented as a clean, modular, header-only C++17 core with a command-line interface driver:

```
EXAMPLE 1 - Story Orchestrator/
├── include/
│   ├── types.hpp              # 11-Tuple Capability, State, Goal, Condition, Effect definitions
│   ├── embedding.hpp          # 5 Standard vector embedding methods & compatibility operators
│   ├── planner.hpp            # A* goal-directed search with vector distance heuristic & safety pruning
│   ├── narrative_engine.hpp   # JSON parsing, state progression, and story template generator
│   └── json.hpp               # Embedded single-header JSON parser (nlohmann::json)
├── src/
│   └── main.cpp               # CLI entry point, argument parsing, vector displays, output generation
├── rabbit_tortoise_story.json # Benchmark dataset 1 (Aesop's Rabbit & Tortoise)
├── the_lion_and_the_mouse.json# Benchmark dataset 2 (Aesop's Lion & Mouse)
├── space_rescue_mission.json  # Benchmark dataset 3 (Mars Rover & Astronaut)
├── teamwork_adventure.json    # Benchmark dataset 4 (Cooperative Obstacle Adventure)
├── story.txt                  # Output artifact of generated narrative
├── Makefile                   # Reproducible standalone build script
├── README.md                  # Domain guide & overview
├── USAGE.md                   # CLI syntax and verification manual
├── DELIVERABLE_1_FORMAL_EMBEDDING_DESIGN.md
├── DELIVERABLE_2_IMPLEMENTATION.md
├── DELIVERABLE_3_EXPERIMENTAL_DATASET.md
└── DELIVERABLE_4_TECHNICAL_REPORT.md
```

### Dependencies
- **Language Standard**: C++17 (`-std=c++17`)
- **Compiler Support**: Clang++ (macOS / Linux) or G++ (Linux / Windows MinGW)
- **External Dependencies**: Zero external runtime libraries. Uses standard library `<vector>`, `<string>`, `<unordered_map>`, `<queue>`, `<cmath>`, `<iostream>`, `<fstream>`.

---

## 2. The 5 Core Standard Vector Embedding Methods

All mathematical vector operations are encapsulated within [`include/embedding.hpp`](include/embedding.hpp):

### Method 1: `encode(const State& s)`
```cpp
std::vector<double> encode(const State& s);
```
- **Description**: Maps world state boolean predicates into a bipolar vector $\phi_S(S) \in \{-1.0, +1.0\}^{d_s}$.
- **Vector Space Dimension**: $d_s$ (number of defined domain propositions).
- **Implementation**:
  ```cpp
  std::vector<double> v(domain_vars.size(), -1.0);
  for (size_t i = 0; i < domain_vars.size(); ++i) {
      if (s.has(domain_vars[i]) && s.get_bool(domain_vars[i])) {
          v[i] = +1.0;
      }
  }
  return v;
  ```

### Method 2: `encode(const Goal& g)`
```cpp
std::vector<double> encode(const Goal& g);
```
- **Description**: Encodes goal requirements with a ternary mask $\phi_G(G) \in \{-1.0, 0.0, +1.0\}^{d_s}$. Unspecified variables receive weight $0.0$, preventing unconstrained state dimensions from penalizing path distance.
- **Vector Space Dimension**: $d_s$.

### Method 3: `encode(const Capability& c)`
```cpp
std::vector<double> encode(const Capability& c);
```
- **Description**: Projects an 11-tuple narrative beat capability into a structured vector $\phi_C(C) \in \mathbb{R}^{2d_s + 4}$.
- **Subspace Layout**:
  - `[0, ds - 1]`: Precondition subspace $\mathbf{v}_{\text{pre}} \in \{-1, 0, +1\}^{d_s}$
  - `[ds, 2*ds - 1]`: Effect subspace $\mathbf{v}_{\text{eff}} \in \{-1, 0, +1\}^{d_s}$
  - `[2*ds]`: $C_{\text{time}}$ (normalized pacing duration)
  - `[2*ds + 1]`: $C_{\text{resource}}$ (prop and character cost)
  - `[2*ds + 2]`: $\operatorname{Rel}_i$ (dramatic coherence probability)
  - `[2*ds + 3]`: $\operatorname{KidFriendly}$ safety flag ($1.0$ if safe, $0.0$ if hazardous)

### Method 4: `compose(const Capability& c1, const Capability& c2)`
```cpp
Capability compose(const Capability& c1, const Capability& c2);
```
- **Description**: Computes the composite capability $C_{12} = C_2 \circ C_1$ where $C_1$ precedes $C_2$.
- **Semantics**:
  - $P(C_{12}) = P(C_1) \cup (P(C_2) \setminus E(C_1))$: Preconditions satisfied by $C_1$ are eliminated.
  - $E(C_{12}) = E(C_1) \oplus E(C_2)$: Effects of $C_2$ override $C_1$.
  - Operational QoS is aggregated: $C_{\text{time}}$ and $C_{\text{resource}}$ are summed; $\operatorname{Rel}$ is multiplied; safety flags are conjunctive.

### Method 5: `similarity(const std::vector<double>& v1, const std::vector<double>& v2)`
```cpp
double similarity(const std::vector<double>& v1, const std::vector<double>& v2);
```
- **Description**: Computes the cosine similarity between two arbitrary vectors:
  $$\operatorname{Sim}(\mathbf{v}_1, \mathbf{v}_2) = \frac{\langle \mathbf{v}_1, \mathbf{v}_2 \rangle}{\|\mathbf{v}_1\|_2 \cdot \|\mathbf{v}_2\|_2 + \epsilon} \in [-1.0, +1.0]$$

---

## 3. Precondition-Effect Compatibility Operator

In addition to the 5 base methods, `embedding.hpp` exposes:
```cpp
double compute_compatibility(const Capability& c1, const Capability& c2);
```
Computing:
$$\operatorname{Comp}(C_1, C_2) = \frac{\langle \mathbf{v}_{\text{eff}}(C_1), \; \mathbf{v}_{\text{pre}}(C_2) \rangle}{\|\mathbf{v}_{\text{pre}}(C_2)\|^2 + \epsilon}$$
Returns $+1.0$ when $C_1$ completely establishes $C_2$'s prerequisites, $-1.0$ when $C_1$ contradicts $C_2$, and $0.0$ when causally unrelated.

---

## 4. Narrative Planner & State Search (`include/planner.hpp`)

The automated narrative orchestrator uses an $A^*$ goal-directed search algorithm:
- **State Representation**: Bipolar state vector $\phi_S(S)$.
- **Goal Condition**: Evaluates whether current state vector satisfies all ternary goal constraints:
  $$\forall k \text{ where } \phi_G(G)_k \neq 0.0: \quad \phi_S(S)_k = \phi_G(G)_k$$
- **Heuristic Function**: Masked Manhattan distance in vector space:
  $$h(S, G) = \sum_{k=1}^{d_s} |\phi_G(G)_k| \cdot \frac{1}{2} \left| \phi_G(G)_k - \phi_S(S)_k \right|$$
  This heuristic is strictly **admissible** and **consistent**, ensuring optimal narrative beat sequences.
- **Safety Pruning**: Any candidate capability leading to a state violating global safety invariants $\mathcal{K}$ (e.g., violent outcomes, cruel endings) is pruned immediately before insertion into the open set.

---

## 5. Narrative Synthesis Engine (`include/narrative_engine.hpp`)

Once the planner discovers the optimal sequence of capabilities $\pi = \langle C_1, C_2, \dots, C_n \rangle$:
1. The engine iterates through each beat, binding narrative tokens and active characters into human-readable prose templates.
2. Formats character dialogue, environmental shifts, dramatic tension arcs, and moral conclusions.
3. Automatically exports the synthesized story to [`story.txt`](story.txt).

---

## 6. Compilation & CLI Execution

### Build Instructions
```bash
# Clean previous builds
make clean

# Compile release binary
make
```

### CLI Command Syntax
```bash
# Basic run on Aesop's Rabbit and Tortoise
./story_orchestrator rabbit_tortoise_story.json

# Display vector space embeddings (Deliverable 1 verification)
./story_orchestrator rabbit_tortoise_story.json --vectors

# Display pairwise precondition-effect compatibility matrix
./story_orchestrator rabbit_tortoise_story.json --compatibility

# Demonstrate algebraic capability composition (C_12 = C_2 o C_1)
./story_orchestrator rabbit_tortoise_story.json --compose

# Full diagnostic mode with all flags enabled
./story_orchestrator rabbit_tortoise_story.json --vectors --compatibility --compose
```
