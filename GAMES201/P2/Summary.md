- [Explicit v.s. implicit time integrators](#explicit-vs-implicit-time-integrators)

# Explicit v.s. implicit time integrators

Explicit (forward Euler, symplectic Euler, RK, ...):

- Future depends only on past
- Easy to implement
- Easy to explode:
$$\Delta{t} \leq c \sqrt{\frac{m}{k}} (c \text{\textasciitilde} 1)$$
- Bad for stiff materials

Implicit (backward Euler, middle-point, ...):

- Future depends on both future and past
- Chicken-egg problem: need to solve a system of (linear) equations
- In general harder to implement
- Each step is more expensive but time steps are larger
  - Sometimes brings you benefits
  - .. but sometimes not
- Numerical damping and locking


Courant-Friedrichs-Lewy (CFL) condition

One upper bound of time step size:

$$C=\frac{u\Delta{t}}{\Delta{x}} \leq C_{max} \text{\textasciitilde} 1$$

- C : CFL number
- $\Delta{t}$ : time step
- $\Delta{x}$ : length interval (e.g. particle radius and grid size)
- u : maximum (velocity)