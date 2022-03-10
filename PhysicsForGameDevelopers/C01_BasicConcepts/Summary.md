- [Basic Concepts](#basic-concepts)
  - [Units and Measures](#units-and-measures)
  - [Mass, Cneter of Mass, and Moment of Inertia](#mass-cneter-of-mass-and-moment-of-inertia)
    - [Calculate each properties](#calculate-each-properties)
    - [Codes](#codes)
    - [parrallel axis theorem](#parrallel-axis-theorem)
    - [Simple geometries mass moments of inertia about the neutral axis](#simple-geometries-mass-moments-of-inertia-about-the-neutral-axis)
  - [Newton's Second Law of Motion](#newtons-second-law-of-motion)
  - [Inertia Tensor](#inertia-tensor)
  - [Relativistic Time](#relativistic-time)


# Basic Concepts

## Units and Measures

![physical_quantities_and_units](./Pictures/physical_quantities_and_units.png)

## Mass, Cneter of Mass, and Moment of Inertia

- mass, mass as a measure of a body's resistance to motion or a change in its motion.
- center of mass, the point through which any force can act on the body without resulting in a rotation of the body.
- moment of inertia(rotational inertia), the mass moment of inertia of a body is a quatitative measure of the radial distribution of the mass of a body about a given axis of rotation, is a measure of a body's resistance to rotational motion.

### Calculate each properties

- mass, $m = \int \rho \space dV = \rho \int dV$, or simply sum the masses of all components to arrive at the total mass.
- center of mass
$$
x_c = \{\sum x_o \space m_i\} / \{\sum m_i\} \\
y_c = \{\sum y_o \space m_i\} / \{\sum m_i\} \\
x_c = \{\sum z_o \space m_i\} / \{\sum m_i\}
$$
total mass and center of gravity for a system
$$
m_t = \sum m_i \\
CG = [\sum (cg_i)(m_i)] / m_t
$$
$cg_i$ is the location of the center of gravity of each point mass in reference coordinates.
- moment of inertia
$$
\bold{I} = \int r^2 dm
$$

### Codes

[mass_calculator.py](./Scripts/mass_calculator.py)

### parrallel axis theorem


$$\bold{I} = \bold{I}_o + md^2$$

where m is the mass of the body and d is the perpendicular distance between the parallel axes. $\bold{I}_o$ the moment of inertia of a body about the neutral axis.

### Simple geometries mass moments of inertia about the neutral axis

![0](./Pictures/mass_moments_of_inertia_0.png)
![1](./Pictures/mass_moments_of_inertia_1.png)
![2](./Pictures/mass_moments_of_inertia_2.png)

## Newton's Second Law of Motion

$$\sum \bold{F} = d\bold{G}/dt = m\bold{a}$$

$$\sum \bold{M}_{cg} = d / dt(\bold{H}_{cg})$$

where $\sum \bold{M}_{cg}$ is the sum of all moments about the body center of gravity, and $\bold{H}$ is the angular momentum of the body.

$$\bold{M}_{cg} = \bold{r} \times \bold{F}$$

$$\bold{H}_{cg}=\sum\bold{r}_i \times m_i(\boldsymbol{\omega} \times \bold{r}_i)=\int (\bold{r} \times (\boldsymbol{\omega} \times \bold{r})) dm$$

i represents the $i$th particle making up the body, $\boldsymbol{\omega}$ is the angular velocity of the body about the axis under consideration, and $\bold{r}_i \times m_i(\boldsymbol{\omega} \times \bold{r}_i)$ is the angular momentum of the $i$th particle.

$$\bold{H}_{cg} = \boldsymbol{\omega} \int  \bold{r}^2 dm = \omega \bold{I}$$

$$d\bold{H}_{cg}/dt = d/dt(\bold{I} \boldsymbol{\omega}) = \bold{I} d\boldsymbol{\omega}/dt = \bold{I} \alpha$$

$\alpha$ is the angular acceleration of the body about a given axis.

$$\sum \bold{M}_{cg} = \bold{I}\alpha$$

local (body) coordinates

$$(d\bold{V}/dt)_{fixed} = (d\bold{V}/dt)_{rot}+(\boldsymbol{\omega} \times \bold{V})$$

$$\sum \bold{M}_{cg} = d\bold{H}_{cg}/dt = \bold{I}(d\boldsymbol{\omega}/dt) + (\boldsymbol{\omega} \times (\bold{I}\boldsymbol{\omega}))$$

## Inertia Tensor

in 3D space

$$
\bold{I} = \begin{pmatrix} I_{xx}, -I_{xy}, -I_{xz}\\
-I_{yx}, I_{yy}, -I_{yz} \\
-I_{zx}, -I_{zy}, I_{zz}
 \end{pmatrix}
$$

## Relativistic Time

Lorentz factor

$$\gamma = \frac{1}{\sqrt{1 - \frac{v^2}{c^2}}}$$