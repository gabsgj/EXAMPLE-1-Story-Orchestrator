# Usage Guide: EXAMPLE 1 - Story Orchestrator

This guide walks through compiling, running, and inspecting the Story Orchestrator.

---

## 1. Building the Program

Ensure you have a modern C++ compiler supporting C++17 (`g++` or `clang++`):

```bash
cd "EXAMPLE 1 - Story Orchestrator"
make
```

To clean build artifacts:
```bash
make clean
```

---

## 2. Command Line Options

The executable accepts a JSON problem file and optional feature flags:

```text
Usage:
  ./story_orchestrator <problem_spec.json> [options]

Options:
  --vectors         Display vector embeddings for states, goals, & capabilities
  --compatibility   Display precondition-effect compatibility matrix
  --compose         Demonstrate formal capability composition (C_2 o C_1)
  --help            Show this help message
```

---

## 3. Example Execution Scenarios

### Scenario A: Standard Story Synthesis
Synthesizes the complete fable and generates `story.txt`:
```bash
./story_orchestrator rabbit_tortoise_story.json
```

### Scenario B: Inspecting Embeddings and Composability
Displays the mathematical vector representations ($\phi_S, \phi_G, \phi_C$), the compatibility matrix, and composite capability $C_{12} = C_2 \circ C_1$:
```bash
./story_orchestrator rabbit_tortoise_story.json --vectors --compatibility --compose
```

### Scenario C: Running Teamwork Problem
```bash
./story_orchestrator teamwork_adventure.json --compatibility
```

### Scenario D: The Lion and the Mouse (Aesop Fable)
```bash
./story_orchestrator the_lion_and_the_mouse.json
```

### Scenario E: Sci-Fi Cooperative Mars Rescue
```bash
./story_orchestrator space_rescue_mission.json
```

---

## 4. Output Artifacts

Upon successful planning and execution, the program creates:
1. `story.txt`: The complete formatted kid-friendly story text with moral synthesis.

