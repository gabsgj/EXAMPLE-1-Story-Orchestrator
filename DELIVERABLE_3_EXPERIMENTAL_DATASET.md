# Deliverable 3: Experimental Datasets
## EXAMPLE 1: Autonomous Story Orchestrator

**Implementation Domain**: Qualitative & Narrative World State Space  
**Directory**: `EXAMPLE 1 - Story Orchestrator/`

---

## 1. Dataset Overview

This implementation includes four comprehensive narrative benchmark datasets in standardized JSON format. Each dataset defines a complete formal application model $\mathcal{A} = (\mathcal{S}, \mathcal{C}, S_I, G, \mathcal{R}, \mathcal{K})$, including state proposition vocabularies, initial world states, narrative goals, capabilities (transitions), and safety invariants.

| Dataset File | Story Domain | Narrative Variables ($d_s$) | Total Capabilities ($|\mathcal{C}|$) | Hazard States ($|\mathcal{K}|$) | Core Moral / Climax |
|:---|:---|:---:|:---:|:---:|:---|
| [`rabbit_tortoise_story.json`](rabbit_tortoise_story.json) | Aesop's Classic Fable | 17 | 10 | 1 | "Slow and steady wins the race" |
| [`the_lion_and_the_mouse.json`](the_lion_and_the_mouse.json) | Aesop's Mercy Fable | 16 | 11 | 1 | "No act of kindness, no matter how small, is ever wasted" |
| [`space_rescue_mission.json`](space_rescue_mission.json) | Sci-Fi Cooperative Mars Rescue | 17 | 9 | 1 | "Teamwork overcomes hostile environments" |
| [`teamwork_adventure.json`](teamwork_adventure.json) | Cooperative Fantasy Puzzle | 15 | 8 | 1 | "Collaboration unlocks locked doors" |

---

## 2. Detailed Dataset Specifications

### 2.1 Benchmark 1: `rabbit_tortoise_story.json`
- **Theme**: Hubris, perseverance, and humility.
- **Characters**: Swift the Rabbit (overconfident), Toby the Tortoise (persistent), Benny the Beaver (referee).
- **State Space Dimensions ($d_s = 17$)**:
  - `rabbit_at_start`, `tortoise_at_start`, `race_started`
  - `rabbit_running`, `rabbit_far_ahead`, `rabbit_mocking`
  - `rabbit_takes_nap`, `rabbit_sleeping`, `tortoise_plodding`
  - `tortoise_passes_rabbit`, `rabbit_wakes_up`, `rabbit_panics`
  - `tortoise_nears_finish`, `tortoise_wins`, `rabbit_humiliated`
  - `lesson_learned`, `cruel_ending`
- **Initial State $S_I$**: `rabbit_at_start = true`, `tortoise_at_start = true`. All other propositions false.
- **Goal State $G$**: `tortoise_wins = true`, `lesson_learned = true`.
- **Key Capabilities**:
  - `announce_race`: Benny the Beaver starts the challenge.
  - `rabbit_dash`: Swift dashes ahead with overwhelming speed.
  - `rabbit_mock`: Swift pauses to mock Toby's steady pace.
  - `rabbit_sleep`: Swift arrogantly decides to take a nap under an oak tree.
  - `tortoise_steady_march`: Toby advances steadily step by step.
  - `tortoise_overtake`: Toby quietly passes the slumbering rabbit.
  - `rabbit_wake_panic`: Swift awakens in terror as Toby approaches the ribbon.
  - `tortoise_cross_finish`: Toby breaks the ribbon to win the race.
  - `rabbit_learns_humility`: Swift congratulates Toby and embraces persistence.
  - `rabbit_cruel_mockery` (*Hazardous Capability*): Violates safety invariant (`cruel_ending = true`), safely pruned by vector space guardrails.

---

### 2.2 Benchmark 2: `the_lion_and_the_mouse.json`
- **Theme**: Mercy, reciprocity, and unexpected alliances.
- **Characters**: Leo the Lion (mighty apex predator), Milo the Mouse (tiny and vulnerable), Hunter (unseen danger).
- **State Space Dimensions ($d_s = 16$)**:
  - `lion_asleep`, `mouse_foraging`, `mouse_wakes_lion`
  - `lion_captures_mouse`, `mouse_pleads_mercy`, `lion_shows_mercy`
  - `mouse_freed`, `hunters_lay_trap`, `lion_trapped_in_net`
  - `lion_roars_in_distress`, `mouse_hears_roar`, `mouse_arrives`
  - `mouse_gnaws_ropes`, `lion_freed`, `friendship_formed`
  - `lion_eats_mouse` (*Hazard state*)
- **Initial State $S_I$**: `lion_asleep = true`, `mouse_foraging = true`.
- **Goal State $G$**: `lion_freed = true`, `friendship_formed = true`.
- **Safety Invariant $\mathcal{K}$**: `lion_eats_mouse = true` is marked as a fatal hazard violating genre constraints. The planner successfully circumvents this path.

---

### 2.3 Benchmark 3: `space_rescue_mission.json`
- **Theme**: Sci-fi planetary exploration, cooperative problem solving, and survival.
- **Entities**: Commander Nova (astronaut trapped in Habitat-B), Rover Sparky (autonomous solar rover), Mars Dust Storm (dynamic environmental hazard).
- **State Space Dimensions ($d_s = 17$)**:
  - `sparky_docked`, `storm_detected`, `comms_offline`
  - `nova_oxygen_low`, `sparky_solar_charged`, `route_planned`
  - `sparky_traversing_crater`, `shelter_found`, `comms_relay_deployed`
  - `habitat_reached`, `spare_o2_transferred`, `power_restored`
  - `life_support_active`, `nova_rescued`, `safe_return`
  - `rover_battery_dead` (*Hazard state*), `mission_failed` (*Hazard state*)
- **Initial State $S_I$**: `sparky_docked = true`, `storm_detected = true`, `nova_oxygen_low = true`, `comms_offline = true`.
- **Goal State $G$**: `nova_rescued = true`, `safe_return = true`.
- **Challenge**: The planner must balance rover power drain ($C_{\text{resource}}$) against life support countdown ($C_{\text{time}}$).

---

### 2.4 Benchmark 4: `teamwork_adventure.json`
- **Theme**: Multi-agent puzzle solving and cooperative locks.
- **Characters**: Aaron the Archer (ranged capabilities), Bran the Builder (heavy lifting capabilities).
- **State Space Dimensions ($d_s = 15$)**:
  - `adventurers_at_gate`, `gate_locked`, `high_lever_visible`
  - `chasm_blocking`, `planks_available`, `bridge_built`
  - `lever_shot_down`, `gate_mechanism_exposed`, `heavy_gear_stuck`
  - `gear_turned`, `gate_open`, `treasure_chamber_reached`
  - `trap_triggered` (*Hazard state*), `party_separated` (*Hazard state*)
- **Initial State $S_I$**: `adventurers_at_gate = true`, `gate_locked = true`, `high_lever_visible = true`, `chasm_blocking = true`, `planks_available = true`.
- **Goal State $G$**: `treasure_chamber_reached = true`.
- **Challenge**: Capabilities have strict causal interdependencies requiring Aaron and Bran to interleave their respective skills.

---

## 3. Dataset JSON Schema

All benchmark JSON files adhere to this rigorous schema:
```json
{
  "application": "StoryOrchestrator",
  "domain": "Domain Name",
  "description": "Problem narrative overview",
  "state_variables": [
    {"name": "var_name", "type": "bool", "description": "Predicate description"}
  ],
  "initial_state": {
    "var_name": true
  },
  "goal": {
    "target_var": true
  },
  "hazard_states": [
    {"var_name": true}
  ],
  "capabilities": [
    {
      "id": "beat_identifier",
      "type": "NARRATIVE_BEAT",
      "name": "Human Readable Name",
      "preconditions": {"var_name": true},
      "effects": {"var_name": true, "other_var": false},
      "qos": {
        "time": 2.5,
        "resource": 1.0,
        "reliability": 0.99,
        "kid_friendly": true
      },
      "story_template": "Narrative paragraph template..."
    }
  ]
}
```
