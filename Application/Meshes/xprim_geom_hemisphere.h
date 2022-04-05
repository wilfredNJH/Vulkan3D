namespace xprim_geom::hemisphere
{
    //-----------------------------------------------------------------------------
    inline
    mesh Generate(int Rings, float RadicalSegments, float Height, float Radius, bool isClose ) noexcept
    {
        mesh Mesh;

        const float Scale = Height;
        int         thisrow = 0;
        int         prevrow = 0;
        int         Point = 0;

        for (int j = 0, end_j = Rings + 1; j <= end_j; ++j)
        {
            const float v = j / float(Rings + 1.0f);
            const float w = std::sinf(pi_v * v);
            const float	y = Scale * std::cosf(pi_v * v);

            for (int i = 0; i <= RadicalSegments; ++i)
            {
                const float u = i / float{ RadicalSegments };
                const float	x = std::sinf(u * tau_v);
                const float	z = std::cosf(u * tau_v);

                if ( y < 0.0f )
                {
                    if (isClose == false) break;

                    Mesh.m_Vertices.push_back
                    (vertex
                        { .m_Position = {x * Radius * w, 0.0, z * Radius * w}
                        , .m_Normal = { 0.0f, -1.0f, 0.0f }
                        , .m_Tangent = { z, 0.0f, -x, 1.0f }
                        , .m_Texcoord = { u, v }
                        });
                }
                else
                {
                    Mesh.m_Vertices.push_back
                    ( vertex
                    { .m_Position = { x * Radius * w, y, z * Radius * w }
                    , .m_Normal   = { x * Radius * w * Scale, y / Scale, z * Radius * w * Scale }
                    , .m_Tangent  = { z, 0.0f, -x, 1.0f }
                    , .m_Texcoord = { u, v }
                    });
                }
                Point++;

                if (i > 0 && j > 0)
                {
                    Mesh.m_Indices.push_back(prevrow + i - 1);
                    Mesh.m_Indices.push_back(thisrow + i - 1);
                    Mesh.m_Indices.push_back(prevrow + i);

                    Mesh.m_Indices.push_back(prevrow + i);
                    Mesh.m_Indices.push_back(thisrow + i - 1);
                    Mesh.m_Indices.push_back(thisrow + i);
                }
            }

            prevrow = thisrow;
            thisrow = Point;
        }

        return Mesh;
    }
}





