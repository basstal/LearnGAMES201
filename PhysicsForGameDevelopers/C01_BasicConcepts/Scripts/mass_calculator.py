import taichi as ti

ti.init(arch=ti.gpu, debug=True)

NUMELEMENTS = 10

mass = ti.field(float, (NUMELEMENTS))
designPosition = ti.Vector.field(3, float, (NUMELEMENTS))
correctedPosition = ti.Vector.field(3, float, (NUMELEMENTS))

TotalMass = ti.field(float, ())
CombinedCG = ti.Vector.field(3, float, ())
FirstMoment = ti.Vector.field(3, float, ())


@ti.func
def next_vector3(scalar: ti.int32):
    return [
        ti.random(float) * scalar,
        ti.random(float) * scalar,
        ti.random(float) * scalar
    ]


@ti.kernel
def mass_calculator():
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

    print("TotalMass = ", TotalMass[None])
    for i in range(NUMELEMENTS):
        print("mass[i] = ", mass[i])
        print("designPosition[i] = ", designPosition[i],
              ", correctedPosition[i] = ", correctedPosition[i])
    print("CombinedCG = ", CombinedCG[None])


mass_calculator()