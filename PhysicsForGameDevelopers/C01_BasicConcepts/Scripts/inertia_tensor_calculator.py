import taichi as ti

ti.init(arch=ti.gpu, debug=True)

NUMELEMENTS = 10

mass = ti.field(float, (NUMELEMENTS))
designPosition = ti.Vector.field(3, float, (NUMELEMENTS))
correctedPosition = ti.Vector.field(3, float, (NUMELEMENTS))
localInertia = ti.Vector.field(3, float, (NUMELEMENTS))

TotalMass = ti.field(float, ())
CombinedCG = ti.Vector.field(3, float, ())
FirstMoment = ti.Vector.field(3, float, ())

Ixx = ti.field(float, ())
Iyy = ti.field(float, ())
Izz = ti.field(float, ())
Ixy = ti.field(float, ())
Ixz = ti.field(float, ())
Iyz = ti.field(float, ())
InertiaTensor = ti.Matrix.field(3, 3, float, ())


@ti.func
def next_vector3(scalar: ti.int32):
    return [
        ti.random(float) * scalar,
        ti.random(float) * scalar,
        ti.random(float) * scalar
    ]


@ti.kernel
def inertia_tensor_calculator():
    for i in range(NUMELEMENTS):
        mass[i] = ti.random(float) * NUMELEMENTS
        designPosition[i].fill(next_vector3(NUMELEMENTS))
    for i in range(NUMELEMENTS):
        TotalMass[None] += mass[i]

    for i in range(NUMELEMENTS):
        FirstMoment[None] += mass[i] * designPosition[i]

    CombinedCG[None] = FirstMoment[None] / TotalMass[None]

    for i in range(NUMELEMENTS):
        correctedPosition[i] = designPosition[i] - CombinedCG[None]

    for i in range(NUMELEMENTS):
        Ixx[None] += localInertia[i].x + mass[i] * (
            correctedPosition[i].y * correctedPosition[i].y +
            correctedPosition[i].z * correctedPosition[i].z)

        Iyy[None] += localInertia[i].y + mass[i] * (
            correctedPosition[i].x * correctedPosition[i].x +
            correctedPosition[i].z * correctedPosition[i].z)

        Izz[None] += localInertia[i].z + mass[i] * (
            correctedPosition[i].x * correctedPosition[i].x +
            correctedPosition[i].y * correctedPosition[i].y)

        Ixy[None] += mass[i] * (correctedPosition[i].x *
                                correctedPosition[i].y)
        Ixz[None] += mass[i] * (correctedPosition[i].x *
                                correctedPosition[i].z)
        Iyz[None] += mass[i] * (correctedPosition[i].y *
                                correctedPosition[i].z)

    InertiaTensor[None][0, 0] = Ixx[None]
    InertiaTensor[None][0, 1] = -Ixy[None]
    InertiaTensor[None][0, 2] = -Ixz[None]
    InertiaTensor[None][1, 0] = -Ixy[None]
    InertiaTensor[None][1, 1] = Iyy[None]
    InertiaTensor[None][1, 2] = -Iyz[None]
    InertiaTensor[None][2, 0] = -Ixz[None]
    InertiaTensor[None][2, 1] = -Iyz[None]
    InertiaTensor[None][2, 2] = Izz[None]

    print("TotalMass = ", TotalMass[None])
    for i in range(NUMELEMENTS):
        print("mass[i] = ", mass[i])
        print("designPosition[i] = ", designPosition[i],
              ", correctedPosition[i] = ", correctedPosition[i], ", ")
    print("InertiaTensor = ", InertiaTensor[None])
    print("CombinedCG = ", CombinedCG[None])


inertia_tensor_calculator()