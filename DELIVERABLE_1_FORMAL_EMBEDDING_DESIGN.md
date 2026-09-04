# Deliverable 1: Formal Embedding Design
## EXAMPLE 1: Autonomous Story Orchestrator

**Implementation Domain**: Qualitative & Narrative World State Space  
**Directory**: `EXAMPLE 1 - Story Orchestrator/`

---

## 1. Mathematical Application Model

The narrative orchestration domain models automated story progression and character interaction as a formal state-transition system. The application environment is formalized as a 6-tuple:

$$\mathcal{A} = (\mathcal{S}, \mathcal{C}, S_I, G, \mathcal{R}, \mathcal{K})$$

Where:
- $\mathcal{S}$: The state space consisting of all valid narrative world configurations over $d_s$ boolean and categorical propositions representing character conditions, locations, knowledge, and relationships.
- $\mathcal{C}$: The finite set of available narrative beat capabilities. In this model, **capabilities are transitions ($\Delta \mathbf{s}$), not states**. Each capability represents an atomic narrative action, event, or revelation that transforms one world state into another.
- $S_I \in \mathcal{S}$: The initial world state representing story exposition (e.g., characters at starting locations, initial disposition, problem unsolved).
- $G$: The goal specification representing narrative climax and resolution (e.g., race concluded, moral lesson internalized, cooperative objective achieved).
- $\mathcal{R}$: Narrative resources (characters, landmarks, story props, pacing budget).
- $\mathcal{K}$: Global safety invariants and moral guardrails (e.g., preventing violent outcomes, avoiding negative morals, maintaining kid-friendly themes).

---

## 2. State, Goal, and Capability Representations

### 2.1 State Representation $\phi_S(S) \in \mathbb{R}^{d_s}$

World state variables are encoded into a continuous vector space using a **bipolar representation**:

$$\phi_S(S) = \begin{bmatrix} s_1 \\ s_2 \\ \vdots \\ s_{d_s} \end{bmatrix}, \quad s_k \in \{-1.0, +1.0\}$$

Where each dimension corresponds to a specific narrative predicate:
$$\phi_S(S)_k = \begin{cases} 
+1.0 & \text{if narrative proposition } p_k = \text{true} \\
-1.0 & \text{if narrative proposition } p_k = \text{false} 
\end{cases}$$

#### Rationale for Bipolar Encoding Over Standard Binary $\{0, 1\}$
In standard $\{0, 1\}$ binary representations, a zero value ($0.0$) creates mathematical ambiguity between:
1. A predicate that is explicitly required to be **false** ($p = \text{false}$).
2. A predicate that is **unconstrained** or irrelevant ("don't-care").

Under standard dot-product metrics, $0 \cdot 1 = 0$ and $0 \cdot 0 = 0$, causing orthogonality to be conflated with negation. Under bipolar encoding $\{-1.0, +1.0\}$:
- Agreement between states yields positive inner products: $(+1)(+1) = 1$ and $(-1)(-1) = 1$.
- Direct contradictions yield negative inner products: $(+1)(-1) = -1$.
- Unconstrained dimensions are cleanly handled by the goal mask.

### 2.2 Goal Representation $\phi_G(G) \in \mathbb{R}^{d_s}$

The narrative goal represents a partial state specification with a ternary masked embedding:

$$\phi_G(G)_k = \begin{cases} 
+1.0 & \text{if goal requires proposition } p_k = \text{true} \\
-1.0 & \text{if goal requires proposition } p_k = \text{false} \\
0.0  & \text{if proposition } p_k \text{ is unconstrained (don't-care)}
\end{cases}$$

This ensures that dot-product similarity or distance between state and goal only accumulates penalties or rewards along dimensions explicitly governed by the narrative objective:

$$\operatorname{Distance}(S, G) = \sum_{k=1}^{d_s} |\phi_G(G)_k| \cdot \left| \phi_G(G)_k - \phi_S(S)_k \right|$$

Dimensions where $\phi_G(G)_k = 0.0$ contribute zero penalty to the heuristic.

### 2.3 11-Tuple Capability Model

Every narrative beat capability $C_i \in \mathcal{C}$ is formalized as an 11-tuple:

$$C_i = (T_i, I_i, O_i, P_i, E_i, K_i, R_i, Q_i, \operatorname{Rel}_i, A_i, M_i)$$

1. **$T_i$ (Capability Type)**: $\code{NARRATIVE\_BEAT}$ representing an atomic dramatic beat.
2. **$I_i$ (Input Ports)**: Required story entities (e.g., active characters, prop handles).
3. **$O_i$ (Output Ports)**: Resulting narrative entities or updated character statuses.
4. **$P_i$ (Preconditions)**: Logical guard conditions required before the beat can occur.
5. **$E_i$ (Effects)**: State transformation rules applied upon beat execution.
6. **$K_i$ (Safety Invariants)**: Hard constraints enforcing genre compliance and moral safety.
7. **$R_i$ (Resource Requirements)**: Character availability and prop ownership.
8. **$Q_i$ (Quality of Service / Pacing)**: Dramatic tension, duration/reading time $C_{\text{time}}$, narrative complexity $C_{\text{resource}}$.
9. **$\operatorname{Rel}_i \in [0, 1]$ (Pacing Reliability)**: Probability that the beat successfully advances audience engagement without narrative confusion.
10. **$A_i \in \{0, 1\}$ (Availability)**: Whether characters involved are currently present and able to perform the beat.
11. **$M_i$ (Mechanism)**: Story synthesis template and moral commentary generator.

---

## 3. Subspace Vector Space Embedding $\phi_C(C) \in \mathbb{R}^{d_c}$

Each narrative capability $C_i$ is mapped into a structured vector space $\mathbb{R}^{d_c}$ where $d_c = 2d_s + 4$:

$$\phi_C(C) = \begin{bmatrix} 
\mathbf{v}_{\text{pre}} \in \mathbb{R}^{d_s} \\ 
\mathbf{v}_{\text{eff}} \in \mathbb{R}^{d_s} \\ 
C_{\text{time}} \\ 
C_{\text{resource}} \\ 
\operatorname{Rel}_i \\ 
\operatorname{KidFriendly} 
\end{bmatrix}$$

### 3.1 Precondition Subspace $\mathbf{v}_{\text{pre}} \in \{-1.0, 0.0, +1.0\}^{d_s}$

$$\mathbf{v}_{\text{pre}}(C)_k = \begin{cases}
+1.0 & \text{if } C \text{ requires proposition } p_k = \text{true} \\
-1.0 & \text{if } C \text{ requires proposition } p_k = \text{false} \\
0.0  & \text{if } C \text{ does not depend on proposition } p_k
\end{cases}$$

### 3.2 Effect Subspace $\mathbf{v}_{\text{eff}} \in \{-1.0, 0.0, +1.0\}^{d_s}$

$$\mathbf{v}_{\text{eff}}(C)_k = \begin{cases}
+1.0 & \text{if } C \text{ asserts } p_k = \text{true} \\
-1.0 & \text{if } C \text{ asserts } p_k = \text{false} \\
0.0  & \text{if } C \text{ leaves proposition } p_k \text{ unchanged}
\end{cases}$$

### 3.3 Quality & Safety Subspace $\mathbf{v}_{\text{qos}} \in \mathbb{R}^4$
Contains continuous pacing metrics:
- $C_{\text{time}} \in \mathbb{R}^+$: Pacing duration (minutes / beats).
- $C_{\text{resource}} \in \mathbb{R}^+$: Character attention cost / prop usage.
- $\operatorname{Rel}_i \in [0.0, 1.0]$: Dramatic coherence score.
- $\operatorname{KidFriendly} \in \{0.0, 1.0\}$: Safety invariant flag (must equal $1.0$ for valid beats).

---

## 4. Precondition-Effect Compatibility Measure

To determine whether narrative beat $C_1$ causally enables narrative beat $C_2$, we compute the normalized directional inner product between the effect vector of $C_1$ and the precondition vector of $C_2$:

$$\operatorname{Comp}(C_1, C_2) = \frac{\langle \mathbf{v}_{\text{eff}}(C_1), \; \mathbf{v}_{\text{pre}}(C_2) \rangle}{\|\mathbf{v}_{\text{pre}}(C_2)\|^2 + \epsilon}$$

Where:
- $\langle \mathbf{a}, \mathbf{b} \rangle = \sum_{k=1}^{d_s} a_k b_k$ is the standard Euclidean inner product.
- $\|\mathbf{v}_{\text{pre}}(C_2)\|^2 = \sum_{k=1}^{d_s} (\mathbf{v}_{\text{pre}}(C_2)_k)^2$ is the number of active preconditions in $C_2$.
- $\epsilon > 0$ is a small stabilization constant preventing division by zero when $C_2$ has no preconditions.

### Semantic Interpretation of $\operatorname{Comp}(C_1, C_2)$:
- **$\operatorname{Comp}(C_1, C_2) > 0$ (Causal Enablement)**: $C_1$ produces one or more conditions directly required by $C_2$. If $\operatorname{Comp}(C_1, C_2) = 1.0$, $C_1$ completely satisfies all preconditions of $C_2$.
- **$\operatorname{Comp}(C_1, C_2) < 0$ (Causal Invalidation / Conflict)**: $C_1$ asserts effects that directly contradict preconditions of $C_2$ (e.g., $C_1$ makes $p_k = \text{false}$ while $C_2$ requires $p_k = \text{true}$).
- **$\operatorname{Comp}(C_1, C_2) = 0$ (Causal Orthogonality)**: $C_1$ neither enables nor conflicts with $C_2$.

---

## 5. Sequential Capability Composition Algebra

When two narrative beats execute in sequence ($C_{12} = C_2 \circ C_1$, where $C_1$ executes first and $C_2$ executes second), the composite capability $C_{12}$ is formed algebraically:

### 5.1 Precondition Composition
The preconditions of the composite beat must include all preconditions of $C_1$, plus any preconditions of $C_2$ that were **not** already satisfied by the effects of $C_1$:

$$P(C_1 \circ C_2) = P(C_1) \cup \left( P(C_2) \setminus E(C_1) \right)$$

In vector terms:
$$\mathbf{v}_{\text{pre}}(C_{12})_k = \begin{cases}
\mathbf{v}_{\text{pre}}(C_1)_k & \text{if } \mathbf{v}_{\text{pre}}(C_1)_k \neq 0 \\
\mathbf{v}_{\text{pre}}(C_2)_k & \text{if } \mathbf{v}_{\text{pre}}(C_1)_k = 0 \text{ and } \mathbf{v}_{\text{eff}}(C_1)_k = 0 \\
0.0 & \text{if } \mathbf{v}_{\text{eff}}(C_1)_k = \mathbf{v}_{\text{pre}}(C_2)_k \quad (\text{satisfied internally})
\end{cases}$$

### 5.2 Effect Composition
The effects of $C_2$ override or augment the effects of $C_1$:

$$E(C_1 \circ C_2) = E(C_1) \oplus E(C_2)$$

In vector terms:
$$\mathbf{v}_{\text{eff}}(C_{12})_k = \begin{cases}
\mathbf{v}_{\text{eff}}(C_2)_k & \text{if } \mathbf{v}_{\text{eff}}(C_2)_k \neq 0 \\
\mathbf{v}_{\text{eff}}(C_1)_k & \text{if } \mathbf{v}_{\text{eff}}(C_2)_k = 0
\end{cases}$$

### 5.3 Quality and Reliability Aggregation
Operational attributes aggregate homomorphically:
- **Pacing Duration**: $C_{\text{time}}(C_{12}) = C_{\text{time}}(C_1) + C_{\text{time}}(C_2)$ (additive)
- **Resource Cost**: $C_{\text{resource}}(C_{12}) = C_{\text{resource}}(C_1) + C_{\text{resource}}(C_2)$ (additive)
- **Coherence Reliability**: $\operatorname{Rel}(C_{12}) = \operatorname{Rel}(C_1) \cdot \operatorname{Rel}(C_2)$ (multiplicative probability)
- **Safety Invariant**: $\operatorname{KidFriendly}(C_{12}) = \min(\operatorname{KidFriendly}(C_1), \operatorname{KidFriendly}(C_2))$ (conjunctive)

This formal design guarantees that any sequence of narrative beats behaves as a single well-defined macro-capability whose causal and operational properties can be verified entirely in vector space.
