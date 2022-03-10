import taichi as ti

ti.init(arch=ti.gpu)

n = 320
pixels = ti.field(float, (n * 2, n))

@ti.kernel
def paint(t : float):
    for i , j in pixels:
        pixels[i, j] = i * 0.001 + j *0.002 + t

paint(0.3)