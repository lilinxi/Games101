#include <iostream>
#include <vector>

#include "CGL/vector2D.h"

#include "mass.h"
#include "rope.h"
#include "spring.h"

namespace CGL {

    Rope::Rope(Vector2D start, Vector2D end, int num_nodes, float node_mass, float k, vector<int> pinned_nodes) {
        // TODO (Part 1): Create a rope starting at `start`, ending at `end`, and containing `num_nodes` nodes.
        Vector2D diff = (end - start) / (num_nodes - 1);
        for (int i = 0; i < num_nodes; i++) {
            this->masses.push_back(new Mass(start + i * diff, node_mass, false));
        }

//        Comment-in this part when you implement the constructor
        for (auto &i : pinned_nodes) {
            this->masses[i]->pinned = true;
        }

        for (int i = 0; i < num_nodes - 1; i++) {
            this->springs.push_back(new Spring(this->masses[i], this->masses[i + 1], k));
        }

    }

    void Rope::simulateEuler(float delta_t, Vector2D gravity) {
        for (auto &s : springs) {
            // TODO (Part 2): Use Hooke's law to calculate the force on a node
            Vector2D lengthV = s->m2->position - s->m1->position;
            double length = lengthV.norm();
            Vector2D force_a_b = s->k * lengthV / length * (length - s->rest_length);
            s->m1->forces += force_a_b;
            s->m2->forces += -force_a_b;
        }

        for (auto &m : masses) {
            if (!m->pinned) {
                // TODO (Part 2): Add the force due to gravity, then compute the new velocity and position
                m->forces += gravity;
                Vector2D a = m->forces / m->mass;
//                显示欧拉法
//                m->position += m->velocity * delta_t;
//                m->velocity += a * delta_t;

//                半隐式欧拉法
                m->velocity += a * delta_t;
                m->position += m->velocity * delta_t;

                // TODO (Part 2): Add global damping

            }

            // Reset all forces on each mass
            m->forces = Vector2D(0, 0);
        }
    }

//    Verlet算法是经典力学（牛顿力学）中非常经典的一种积分方法，是对牛顿第二定律（运动方程）在计算机上运用的一种数值积分方法，在力学计算运用十分普遍，比如分子运动/模拟（Molecular Dynamics/Simulation），行星运动，等等。
    void Rope::simulateVerlet(float delta_t, Vector2D gravity) {
        for (auto &s : springs) {
            // TODO (Part 3): Simulate one timestep of the rope using explicit Verlet （solving constraints)
            Vector2D lengthV = s->m2->position - s->m1->position;
            double length = lengthV.norm();
            Vector2D force_a_b = s->k * lengthV / length * (length - s->rest_length);
            s->m1->forces += force_a_b;
            s->m2->forces += -force_a_b;
        }

        for (auto &m : masses) {
            if (!m->pinned) {
                Vector2D temp_position = m->position;
                // TODO (Part 3.1): Set the new position of the rope mass
                m->forces += gravity;
                Vector2D a = m->forces / m->mass;
//                无阻尼
//                m->position = 2 * m->position - m->last_position + a * delta_t * delta_t;
//                有阻尼
                double damp_factor = 0.00005;
                m->position =
                        m->position + (1 - damp_factor) * (m->position - m->last_position) + a * delta_t * delta_t;
                m->last_position = temp_position;
                m->forces = Vector2D(0, 0);

                // TODO (Part 4): Add global Verlet damping
            }
        }
    }
}
