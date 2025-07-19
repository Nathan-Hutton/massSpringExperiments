#pragma once

#include <GL/glew.h>
#include <vector>

struct MassSpringPoint
{
	glm::vec3 m_position{ 0.0f };
	glm::vec3 m_velocity{ 0.0f };
	bool fixed{ false };
};

struct Spring
{
	const size_t i, j;
	const float restLength;
	const float stiffness;
};

class MassSpringObject
{
    public:
        MassSpringObject()
        {
			m_points = 
			{
				{ // bottom left
					glm::vec3{ -5.0f, -5.0f, 0.0f },
					glm::vec3{ 0.0f, -0.01f, 0.0f },
					false
				},
				{ // bottom right
					glm::vec3{ 5.0f, -5.0f, 0.0f },
					glm::vec3{ 0.0f, -0.01f, 0.0f },
					false
				},
				{ // top left
					glm::vec3{ -5.0f, 5.0f, 0.0f },
					glm::vec3{ 0.0f },
					true
				},
				{ // top right
					glm::vec3{ 5.0f, 5.0f, 0.0f },
					glm::vec3{ 0.0f },
					true
				}
			};

			// Make springs
			for (size_t i{ 0 }; i < m_points.size(); ++i)
			{
				for (size_t j{ 0 }; j < m_points.size(); ++j)
				{
					if (i == j)
						continue;

					m_springs.emplace_back(Spring{ i, j, glm::length(m_points[i].m_position - m_points[j].m_position), 1.0f });
				}
			}

            m_vertices.resize(m_points.size());
			m_indices = 
			{
				0, 1, 2,
				2, 1, 3
			};

			GLuint EBO;
            glGenVertexArrays(1, &m_VAO);
            glBindVertexArray(m_VAO);

            glGenBuffers(1, &EBO);
            glGenBuffers(1, &m_VBO);
            glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_vertices.size() * 3, m_vertices.data(), GL_DYNAMIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (void*)0);
            glEnableVertexAttribArray(0);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * m_indices.size(), m_indices.data(), GL_STATIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }

		void updatePoints()
		{
			for (MassSpringPoint& point : m_points)
			{
				if (point.fixed)
					continue;

				point.m_position.y -= 0.01f;
			}
		}

		void updateVBO()
		{
			for (size_t i{ 0 }; i < m_points.size(); ++i)
				m_vertices[i] = m_points[i].m_position;

			glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * m_vertices.size() * 3, m_vertices.data());
		}
		
        void draw() const
        {
            glBindVertexArray(m_VAO);
            glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
        }

    private:
		// Physics
		std::vector<MassSpringPoint> m_points{};
		std::vector<Spring> m_springs{};

		// Rendering
        std::vector<glm::vec3> m_vertices{};
		std::vector<GLuint> m_indices{};
        GLuint m_VAO, m_VBO;
};
