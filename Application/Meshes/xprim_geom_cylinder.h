namespace xprim_geom::cylinder
{
    //-----------------------------------------------------------------------------
    inline
    mesh Generate( int Rings, int RadialSegments, float Height, float TopRadius, float BottomRadius ) noexcept
    {
        mesh Mesh;

        int Point   = 0;
        int thisrow = 0;
        int prevrow = 0;
        for( int j = 0, end_j = Rings + 1; j <= end_j; ++j )
        {
            const float v		= j / (Rings + 1.0f);
            const float Radius	= TopRadius + ((BottomRadius - TopRadius) * v);
            const float y       = (Height * 0.5f) - (Height * v);

            for( int i = 0; i <= RadialSegments; ++i ) 
            {
                const float  u = i/(float)RadialSegments;
                const float  x = std::sinf(u * tau_v);
                const float  z = std::cosf(u * tau_v);
                const float3 p = float3( x * Radius, y, z * Radius );

                Mesh.m_Vertices.push_back
                ( vertex
                { .m_Position = p
                , .m_Normal   = { x, 0.0f, z }
                , .m_Tangent  = { z, 0.0f, -x, 1.0f }
                , .m_Texcoord = { u, v * 0.5f }
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
                };
            };

            prevrow = thisrow;
            thisrow = Point;
        }

        // add top
        if( TopRadius > 0.0 ) 
        {
            const float y = Height * 0.5f;
            
            Mesh.m_Vertices.push_back
            ( vertex
            { .m_Position = { 0.0f, y, 0.0f }
            , .m_Normal   = { 0.0f, 1.0f, 0.0f }
            , .m_Tangent  = { 1.0f, 0.0f, 0.0f, 1.0f }
            , .m_Texcoord = { 0.25f, 0.75f }
            });

            thisrow = Point;
            Point++;
            for( int i = 0; i <= RadialSegments; ++i ) 
            {
                const float r = i/(float)RadialSegments;
                const float x = std::sinf( r * tau_v );
                const float z = std::cosf( r * tau_v );
                const float u = ((x + 1.0f) * 0.25f);
                const float v = 0.5f + ((z + 1.0f) * 0.25f);

                Mesh.m_Vertices.push_back
                ( vertex
                { .m_Position = { x * TopRadius, y, z * TopRadius }
                , .m_Normal   = { 0.0f, 1.0f, 0.0f }
                , .m_Tangent  = { 1.0f, 0.0f, 0.0f, 1.0f }
                , .m_Texcoord = { u, v }
                });
                Point++;

                if (i > 0) 
                {
                    Mesh.m_Indices.push_back(thisrow);
                    Mesh.m_Indices.push_back(Point - 2);
                    Mesh.m_Indices.push_back(Point - 1);
                }
            }
        }

        // add bottom
        if( BottomRadius > 0.0 ) 
        {
            const float y = Height * -0.5f;

            Mesh.m_Vertices.push_back
            ( vertex
            { .m_Position = { 0.0f, y, 0.0f }
            , .m_Normal   = { 0.0f, -1.0f, 0.0f }
            , .m_Tangent  = { 1.0f, 0.0f, 0.0f, 1.0f }
            , .m_Texcoord = { 0.25f, 0.75f }
            });

            thisrow = Point;
            Point++;
            for( int i = 0; i <= RadialSegments; ++i ) 
            {
                const float r = i/(float) RadialSegments;
                const float x = std::sinf(r * tau_v);
                const float z = std::cosf(r * tau_v);
                const float u = 0.5f + ((x + 1.0f) * 0.25f);
                const float v = 1.0f - ((z + 1.0f) * 0.25f);

                Mesh.m_Vertices.push_back
                ( vertex
                { .m_Position = { x * BottomRadius, y, z * BottomRadius }
                , .m_Normal   = { 0.0f, -1.0f, 0.0f }
                , .m_Tangent  = { 1.0f, 0.0f, 0.0f, 1.0f }
                , .m_Texcoord = { u, v }
                });

                Point++;

                if (i > 0) 
                {
                    Mesh.m_Indices.push_back(thisrow);
                    Mesh.m_Indices.push_back(Point - 1);
                    Mesh.m_Indices.push_back(Point - 2);
                }
            }
        }

        return Mesh;
    }
}



