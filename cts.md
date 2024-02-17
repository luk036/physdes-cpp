# Clock tree synthesis (CTS)

- also named as Clock routing
- Deferred Merge Embedding Algorithm

---

## ASIC Design flow

Placement -> STA -> Clock skew scheduling -> CTS -> Routing

---

## How to make it easy

- Rectilinear
- Integer coordinate
- Use recursion
- Use diagram

---

## Problem formulation

- Given a set of clock pin locations of registers and a clock source location
- Create a clock tree that meets the balancing requirement, or
- Create a clock tree that meets the skew requirement
- Minimize total wirelength (power)

- Minimize cross-talk

---

## Tree topology generation

- Top down (partition)
    - MMM (Method of means and medians)
    - BB (balanced bipartition)
- Bottom up
    - Matching based
- Enumerate all binary trees
- Local refinement
- Simulated Annealing

- Timing driven

---

## Tapping-point position

- Deferred Merge Embedding (DME) algorithm
- Delay models
  - Linear delay
  - Elmore delay

min_dist() - minimum distance
enlarge()
intersection()



