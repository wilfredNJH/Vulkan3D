namespace xprim_geom::grid
{
    //-----------------------------------------------------------------------------
    inline
    mesh Generate( int SubdivideD, int SubdivideW, float2 Size, float3 CenterOffset ) noexcept
    {
        mesh Mesh;

        // top + bottom 
        const float2    StartPos    = { Size.m_X * -0.5f, Size.m_Y * -0.5f };
        int             Point       = 0;
        int             thisrow     = 0;
        int             prevrow     = 0;
        float           z           = StartPos.m_Y;
        for( int j = 0, end_j = SubdivideD + 1; j <= end_j; ++j )
        {
            float x = StartPos.m_X;
            for( int i = 0, end_i = SubdivideW + 1; i <= end_i; ++i )
            {
                const float u = i/(SubdivideW + 1.0f);
                const float v = j/(SubdivideD + 1.0f);

                Mesh.m_Vertices.push_back
                ( vertex
                { .m_Position = float3(-x, 0.0f, -z) + CenterOffset
                , .m_Normal   = { 0.0f, 1.0f, 0.0f }
                , .m_Tangent  = { 1.0f, 0.0f, 0.0f, 1.0f }
                , .m_Texcoord = { 1.0f - u, 1.0f - v }    // 1.0 - uv to match orientation with Quad
                });

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

                x += Size.m_X / (SubdivideW + 1.0f);
            }

            z += Size.m_Y / (SubdivideD + 1.0f);
            prevrow = thisrow;
            thisrow = Point;
        }

        return Mesh;
    }
}



