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
				{ -5.0f, -5.0f, 0.0f }, // bottom left
				{ 5.0f, -5.0f, 0.0f }, // bottom right
                { -5.0f, 5.0f, 0.0f}, // top left
				{ -5.0f, 5.0f, 0.0f }, // top left
				{ 5.0f, -5.0f, 0.0f }, // bottom right
				{ 5.0f, 5.0f, 0.0f } // top right
            };

            glGenVertexArrays(1, &m_VAO);
            glBindVertexArray(m_VAO);

            glGenBuffers(1, &m_VBO);
            glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_vertices.size() * 3, m_vertices.data(), GL_DYNAMIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), (void*)0);
            glEnableVertexAttribArray(0);

            glBindVertexArray(0);
        }

		void updateVBO()
		{
			glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * m_vertices.size() * 3, m_vertices.data());
		}
		
		void updateVertices()
		{
			for (glm::vec3& vertex : m_vertices)
				vertex.x += 0.01f;
		}

        void draw() const
        {
            glBindVertexArray(m_VAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

    private:
        std::vector<glm::vec3> m_vertices{};
        GLuint m_VAO, m_VBO;
};
