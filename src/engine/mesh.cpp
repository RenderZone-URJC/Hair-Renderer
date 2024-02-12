#include "mesh.h"

namespace glib
{

    int Mesh::INSTANCED_MESHES = 0;

    void Mesh::set_geometry(Geometry &g)
    {
        m_geometry = g;
        m_geometry_loaded = true;
    }
    void Mesh::generate_buffers()
    {
        if(!m_geometry_loaded) return;

        GL_CHECK(glGenVertexArrays(1, &m_vao));

        size_t vertexSize = sizeof(Vertex);

        GL_CHECK(glBindVertexArray(m_vao));

        // [ATTENTION ATTENTION]
        // INTERLEAVED ATTRIBUTES

        unsigned int vbo;
        GL_CHECK(glGenBuffers(1, &vbo));
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vbo));
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, vertexSize * m_geometry.vertices.size(), m_geometry.vertices.data(), GL_STATIC_DRAW));

        // Position attribute
        GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertexSize, (void *)0));
        GL_CHECK(glEnableVertexAttribArray(0));
        // Normal attribute
        GL_CHECK(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vertexSize, (void *)(3 * sizeof(float))));
        GL_CHECK(glEnableVertexAttribArray(1));
        // Tangent attribute
        GL_CHECK(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, vertexSize, (void *)(6 * sizeof(float))));
        GL_CHECK(glEnableVertexAttribArray(2));
        // UV attribute
        GL_CHECK(glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, vertexSize, (void *)(9 * sizeof(float))));
        GL_CHECK(glEnableVertexAttribArray(3));
        // Color attribute
        GL_CHECK(glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(11 * sizeof(float))));
        GL_CHECK(glEnableVertexAttribArray(4));

        if (!m_geometry.indices.empty())
        {
            ASSERT(sizeof(GLuint) == sizeof(unsigned int));
            unsigned int ibo;
            GL_CHECK(glGenBuffers(1, &ibo));
            GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
            GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_geometry.indices.size() * sizeof(m_geometry.indices[0]), m_geometry.indices.data(), GL_STATIC_DRAW));

            m_indexed = true;
        }
        else
        {
            m_indexed = false;
        }

        GL_CHECK(glBindVertexArray(0));
        m_buffer_loaded = true;
    }

    void Mesh::draw(GLenum drawingPrimitive)
    {
        if (m_enabled && m_buffer_loaded)
        {
            GL_CHECK(glBindVertexArray(m_vao));

            if (m_indexed == true)
            {
                GL_CHECK(glDrawElements(drawingPrimitive, m_geometry.indices.size(), GL_UNSIGNED_INT, (void *)0));
            }
            else
            {

                GL_CHECK(glDrawArrays(drawingPrimitive, 0, m_geometry.vertices.size()));
            }
        }
        else
            generate_buffers();
    }

}