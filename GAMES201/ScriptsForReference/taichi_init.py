import taichi as ti

ti.init()

a = ti.field(float, (42, 63))
b = ti.Vector.field(3, float, 4)
c = ti.Matrix.field(2, 2, float, (3, 5))
loss = ti.field(float, ())

a[3, 4] = 1
print('a[3, 4] = ', a[3, 4])

b[2] = [6, 7, 8]
print('b[0] = ', b[0][0], b[0][1], b[0][2])

loss[None] = 3
print(loss[None])

# @ti.kernel
# def fill():
#     for i  in range(10): # Parallelized
#         x[i] += i

#         s = 0
#         for j in range(5):
#             s += j
#         y[i] = s

# @ti.kernel
# def fill_3d():
#     for i, j, k in ti.ndrange((3, 8), (1, 6), 9):
#         x[i, j, k]  = i + j + k

