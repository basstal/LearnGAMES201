- [Collisions](#collisions)
  - [Impulse-Momentum Principle](#impulse-momentum-principle)
  - [Impact](#impact)
  - [Linear and Angular Impulse](#linear-and-angular-impulse)
  - [Friction](#friction)


# Collisions

## Impulse-Momentum Principle

For problems involving constant mass and moment of inertia, you can calculate the average impulse force and torque using the following equations:

$$\bold{F} = m(\bold{v}_+ - \bold{v}_-) / (t_+ - t_-)$$

$$\bold{M} = \bold{I}(\boldsymbol{\omega}_+ - \boldsymbol{\omega}_-) / (t_+ - t_-)$$

## Impact

$$m_1v_{1-}+m_2v_{2-} = m_1v_{1+} + m_2v_{2+}$$

Subscript - refers to the instant just prior to impact, + refers to the instant just after impact.


$$e = -(v_{1+} - v_{2+}) / (v_{1-} - v_{2-})$$

Here e is known as the coefficient of restitution and is a function of the colliding objects' material, construction, and geometry. For collisions that are neither perfectly inelastic(e is 0) nor perfectly elastic(e is 1) e can be any value between 0 and 1.

## Linear and Angular Impulse

$$|\bold{J}| = m(|\bold{v}_{+}| - |\bold{v}_{-}|)$$

$|\bold{J}|$ represent the impluse, let $|\bold{v}_r| = (|\bold{v}_{1-}| - |\bold{v}_{2-}|)$, through equation solve, we get

$$|\bold{J}| = -|\bold{v}_r|(e+1)/(1/m_1 + 1/m_2)$$

$$\bold{v}_{1+} = \bold{v}_{1-} + (|\bold{J}|\bold{n})/m_1$$

$$\bold{v}_{2+} = \bold{v}_{2-} + (-|\bold{J}|\bold{n})/m_2$$

for $|\bold{J}|$ that takes into account both linear and angular effects

$$|\bold{J}| = -(\bold{v}_r \cdot \bold{n})(e+1)/[1/m_1 + 1/m_2 + \bold{n} \cdot ((\bold{r}_1 \times \bold{n}) / \bold{I}_1) \times \bold{r}_1 + \bold{n} \cdot ((\bold{r}_2 \times \bold{n} )/ \bold{I}_2) \times \bold{r}_2]$$

Here $\bold{v}_r$ is the relative velocity along the line of action at the impact point P, and $\bold{n}$ is a unit vector along the line of action at the impact point pointing out from body 1.

$$\boldsymbol{\omega}_{1+} = \boldsymbol{\omega}_{1−} + (\bold{r}_1 \times |\bold{J}| \bold{n})/\bold{I}_1$$

$$\boldsymbol{\omega}_{2+} = \boldsymbol{\omega}_{2−} + (\bold{r}_2 \times -|\bold{J}| \bold{n})/\bold{I}_2$$

## Friction

$$\mu_k = F_f / F_n$$

Here, $F_f$ is the tangential friction force and $F_n$ is the normal impact force. 

$$\omega_+=(Impulse)(\mu r)/I_{cg} + \omega_-$$

$$\bold{v}_{1+}=\bold{v}_{1-}+(J\bold{n} + (\mu J) \bold{t})/ m_1$$

$$\boldsymbol{\omega}_{1+}=\boldsymbol{\omega}_{1-} + (\bold{r}_1 \times (J\bold{n} + (\mu J)\bold{t})) / \bold{I}_{cg}$$

$\bold{t}$ is the unit tangent vector, which is tangent to the collision surfaces and at a right angle to the unit normal vector.

$$\bold{t} = ((\bold{n} \times \bold{v}_r) \times \bold{n}) / |\bold{t}|$$