# 🧠 CSC3050 Project 4 — Cache Design & Side-Channel Analysis

## Global parameter bounds (all test cases):
- Block size: integer in [4, 512] bytes.
- Associativity: one of {1, 2, 4, 8, 16}.
- Number of sets: one of {1, 2, 4, 8, 16, 32, 64, 128, 256}.
- Replacement policy: {LRU, LFU}.
- Write policy: one of (1) write-back & write-allocation, (2) write-through & writeallocation,
(3) write-through & write-no-allocation.

---

### a. Block Size Inference

#### Unconstrained Test Cases (1–8)
- Used **stride-based probing**
- Binary search over stride sizes
- The smallest stride producing **zero hits** corresponds to the cache block size
- A more accurate approach

#### Constrained Test Cases (9–11)
Using `read_1024(0, 1)`:
- The first access causes a miss and loads a full block
- Subsequent accesses within the block are hits

Estimated hit count:
Hits ≈ ((n − 1) / n) × 1024

Derived block size:
Block size ≈ 1024 / (1024 − hits)

**Limitations:**
- Accurate mainly for **power-of-two** or **small block sizes**
- Misalignment effects become significant for large block sizes

---

### b. Associativity Inference
Associativity was inferred by **forcing conflict misses within a single cache set**.

#### Key Probes:
- **Stride = 2³⁰**  
  - Differentiates associativity `< 4` vs `≥ 4`
- **Stride = 2³¹**  
  - Distinguishes between associativity `1` and `2`
- **Stride = 2²⁸**  
  - Differentiates among associativity `4`, `8`, and `16`

Hit/miss patterns directly reflect the cache’s capacity to retain competing blocks.

---

### c. Set Count
⚠️ Due to time constraints, **set count inference was not implemented**.

---

### d. Replacement Policy Detection (LRU vs LFU)

#### Experimental Design
1. **High-Frequency Access**
   - `read_1024(0, 0)` repeatedly accesses one block
2. **Set Saturation**
   - `read_1024(0, 1 << 27)` loads 32 unique blocks into the same set

#### Observations
- **LRU**:
  - High-frequency block is evicted due to lack of recent access
  - Results in all misses after initial load
- **LFU**:
  - High-frequency block remains due to high access count
  - Produces consistent cache hits

This allows reliable differentiation between **recency-based** and **frequency-based** policies.

---

### e. Write Policy Identification
Write policies were inferred using **latency measurements** under mixed hit/miss workloads.

- **Write-back & Write-allocate**
  - Lower latency on cache hits
- **Write-through & Write-allocate**
  - Higher latency due to synchronous memory updates
- **Write-through & No-allocate**
  - Identical latency for hits and misses
  - Reduced read hit rate after write misses

---

### f. Block Size, Set Count, Write Policy
⚠️ Due to time constraints, **set count inference was not completed**.

---

## 📄 Notes
- All experiments were conducted in a **single-process environment**
- Data consistency benefits cannot be fully demonstrated without multiprocessor support
- Focus was placed on **latency behavior and hit/miss analysis**

---

## 🎓 Academic Context
This project was completed as part of **CSC3050 — Computer Architecture** coursework.  
All side-channel experiments were conducted strictly for **educational and analytical purposes**.
