# Adaptive trait dynamics in a changing environment

## Overview

This repository implements an individual-based evolutionary simulation in C++.

A population is characterised by a continuous trait. Individual fitness depends on the match between trait values and an environmental optimum. The environment changes through time, and offspring inherit parental trait values with mutation.

The model is designed as a minimal simulation of adaptation under environmental change.

## Model

At each generation:

1. the environmental optimum is updated
2. fitness is calculated for all individuals as a function of trait–environment mismatch
3. parents are sampled with probability proportional to fitness
4. offspring inherit the parental trait with Gaussian mutation
5. the next generation replaces the previous one

Fitness is defined as:

\[
w(z, E) = \exp(-(z - E)^2)
\]

where:
- \( z \) is the individual trait
- \( E \) is the environmental state

## Output

The simulation records:

- generation
- mean trait value
- mean fitness
- environmental state

These are written to `data/trait_output.csv`.

## Compilation

```bash
g++ -O2 -std=c++17 src/main.cpp -o adaptive_trait_dynamics
