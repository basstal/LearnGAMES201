import taichi as ti
import numpy as np

ti.init(arch=ti.gpu, debug=True)

simulation_steps = 1000
simulation_count = 0

PI = 3.1415926
g = 9.8
t = 0
delta_step = 0.016

lines_start = []
lines_end = []

# Canvas
aspect_ratio = 1.0
image_width = 800
image_height = int(image_width / aspect_ratio)
canvas = ti.Vector.field(3, float, shape=(image_width, image_height))

# Rendering parameters
samples_per_pixel = 4
max_depth = 10

vm = ti.field(float, ())
alpha = ti.field(float, ())
gamma = ti.field(float, ())
b_len = ti.field(float, ())
y_b = ti.field(float, ())
paused = ti.field(ti.i32, ())

target_position = ti.Vector.field(3, float, ())
target_size = ti.Vector.field(3, float, ())

gui = ti.GUI('Particle Kinematics',
             res=(image_width, image_height),
             background_color=0xdddddd)
canvas.fill(0)

vm[None] = 50
alpha[None] = 25
gamma[None] = 8
b_len[None] = 65
y_b[None] = 10

target_position[None] = ti.Vector([400, 75, 45])
target_size[None] = ti.Vector([60, 80, 50])

s = ti.Vector.field(3, float, ())
v = ti.Vector.field(3, float, ())
a = ti.Vector.field(3, float, ())

cos_alpha = ti.cos(alpha[None] / 180 * PI)
b = b_len[None] * ti.cos(PI / 2 - alpha[None] / 180 * PI)
Lx = b * ti.cos(gamma[None] / 180 * PI)
Ly = b_len[None] * cos_alpha
Lz = b * ti.sin(gamma[None] / 180 * PI)


def to_view_space_2d_side(p):
    return (p[0] / image_width, p[1] / image_height)


def to_view_space_2d_top(p):
    return (p[0] / image_width, 0.8 - p[2] / image_height)


def simulate(delta: float):
    global t, simulation_count, cos_alpha
    simulation_count += 1
    t += delta
    cos_theta_x = Lx / b_len[None]
    cos_theta_y = Ly / b_len[None]
    cos_theta_z = Lz / b_len[None]
    s[None] = ti.Vector([
        Lx + vm[None] * cos_theta_x * t,
        (y_b[None] + b_len[None] * cos_alpha) + (vm[None] * cos_theta_y) * t -
        g * (t**2) / 2, Lz + vm[None] * cos_theta_z * t
    ])
    v[None] = ti.Vector([
        vm[None] * cos_theta_x, (vm[None] * cos_theta_y) - g * t,
        vm[None] * cos_theta_z
    ])
    a[None] = -g * ti.Vector([0, 1, 0])


while gui.running:
    for e in gui.get_events(ti.GUI.PRESS):
        if e.key in [ti.GUI.ESCAPE, ti.GUI.EXIT]:
            exit()
        elif e.key == gui.SPACE:
            paused[None] = not paused[None]
    #     elif e.key == ti.GUI.LMB:
    #         new_particle(e.pos[0], e.pos[1])
    #     elif e.key == 'c':
    #         num_particles[None] = 0
    #         rest_length.fill(0)
    #     elif e.key == 's':
    #         if gui.is_pressed('Shift'):
    #             spring_stiffness[None] /= 1.1
    #         else:
    #             spring_stiffness[None] *= 1.1
    #     elif e.key == 'd':
    #         if gui.is_pressed('Shift'):
    #             damping[None] /= 1.1
    #         else:
    #             damping[None] *= 1.1

    # if not paused[None]:
    #     for step in range(10):
    #         substep()

    # X = x.to_numpy()
    # gui.circles(X[:num_particles[None]], color=0xffaa77, radius=5)
    if simulation_count < simulation_steps and not paused[None]:
        start = s[None]
        simulate(delta_step)
        end = s[None]
        # print("start = ", start, ", end = ", end)

    # gui.clear(bg_color)

    # draw side view
    gui.rect(topleft=to_view_space_2d_side(target_position[None] -
                                           target_size[None] / 2),
             bottomright=to_view_space_2d_side(target_position[None] +
                                               target_size[None] / 2),
             color=0x445566,
             radius=1)

    gui.line(begin=to_view_space_2d_side((0, 0)),
             end=to_view_space_2d_side((Lx, (y_b[None] + b_len[None] * cos_alpha))), color=0xFF, radius=2)
    lines_start.append(to_view_space_2d_side(start))
    lines_end.append(to_view_space_2d_side(end))
    gui.lines(np.array(lines_start), np.array(lines_end), color=0x445566)

    # draw top view
    gui.rect(topleft=to_view_space_2d_top(target_position[None] -
                                           target_size[None] / 2),
             bottomright=to_view_space_2d_top(target_position[None] +
                                               target_size[None] / 2),
             color=0x445566,
             radius=1)

    gui.line(begin=to_view_space_2d_top((0, 0, 0)),
             end=to_view_space_2d_top((Lx, 0, Lz)), color=0xFF, radius=2)
    lines_start.append(to_view_space_2d_top(start))
    lines_end.append(to_view_space_2d_top(end))
    gui.lines(np.array(lines_start), np.array(lines_end), color=0x445566)
    # for i in range(num_particles[None]):
    #     for j in range(i + 1, num_particles[None]):
    #         if rest_length[i, j] != 0:
    #             gui.line(begin=X[i], end=X[j], radius=2, color=0x445566)
    # gui.text(content=f'C: clear all; Space: pause', pos=(0, 0.95), color=0x0)
    # gui.text(content=f'S: Spring stiffness {spring_stiffness[None]:.1f}', pos=(0, 0.9), color=0x0)
    # gui.text(content=f'D: damping {damping[None]:.2f}', pos=(0, 0.85), color=0x0)
    gui.show()