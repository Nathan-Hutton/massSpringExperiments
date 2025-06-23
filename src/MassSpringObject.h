#pragma once

#include <GL/glew.h>
#include <vector>

class MassSpringObject
{
    public:
        MassSpringObject()
        {
            m_vertices = 
            {
                -5.0f, -5.0f, 0.0f,
                5.0f, -5.0f, 0.0f,
                0.0f, 5.0f, 0.0f
            };

            glGenVertexArrays(1, &m_VAO);
            glBindVertexArray(m_VAO);

            glGenBuffers(1, &m_VBO);
            glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_vertices.size(), m_vertices.data(), GL_DYNAMIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (void*)0);
            glEnableVertexAttribArray(0);

            glBindVertexArray(0);
        }

        void draw() const
        {
            glBindVertexArray(m_VAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }

    private:
        std::vector<float> m_vertices{};
        GLuint m_VAO, m_VBO;
};
