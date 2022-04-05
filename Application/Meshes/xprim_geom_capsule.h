namespace xprim_geom::capsule
{
    //-----------------------------------------------------------------------------
    inline
    mesh Generate( const int Rings, const int RadialSegments, const float Radius, const float Height ) noexcept
    {
        constexpr auto onethird_v  = 1.0f / 3.0f;
        constexpr auto twothirds_v = 2.0f / 3.0f;

        mesh Mesh;

        Mesh.m_Vertices.reserve( (Rings+2)*(RadialSegments+1)*3 );
        Mesh.m_Indices.reserve ( Mesh.m_Vertices.size()*6 );

        //
        // top hemisphere
        //
        int thisrow = 0;
        int prevrow = 0;
        int point   = 0;

        for( int j=0, end_j = Rings+1; j <= end_j; ++j )
        {
            const float v = j / (float)(Rings + 1);
            const float w = std::sinf( half_pi_v * v );
            const float y = std::cosf( half_pi_v * v ) * Radius;

            for( int i = 0; i <= RadialSegments; ++i ) 
            {
                const float u = i / (float)RadialSegments;
                const float x = -std::sinf(u * tau_v);
                const float z = std::cosf(u * tau_v);
                const auto  p = float3
                {  x * Radius * w
                ,  y
                , -z * Radius * w
                };

                Mesh.m_Vertices.push_back
                ( vertex
                {	.m_Position{ p + float3(0.0f, 0.5f * Height - Radius, 0.0f) }
                ,	.m_Normal  { p.Normalize() }
                ,	.m_Tangent { -z, -0.0f, -x, 1.0f }
                ,	.m_Texcoord{ u, v * onethird_v }
                });

                point++;

                if( i > 0 && j > 0 ) 
                {
                    Mesh.m_Indices.push_back( prevrow + i - 1 );
                    Mesh.m_Indices.push_back( thisrow + i - 1 );
                    Mesh.m_Indices.push_back( prevrow + i     );

                    Mesh.m_Indices.push_back( prevrow + i     );
                    Mesh.m_Indices.push_back( thisrow + i - 1 );
                    Mesh.m_Indices.push_back( thisrow + i     );
                }
            }

            prevrow = thisrow;
            thisrow = point;
        }

        //
        // Cylinder 
        //
        thisrow = point;
        prevrow = 0;
        for( int j = 0, end_j = Rings + 1; j <= end_j; ++j ) 
        {
            const float v = j/(float)(Rings + 1);
            const float y = (0.5f * Height - Radius) - (Height - 2.0f * Radius) * v;

            for( int i = 0; i <= RadialSegments; ++i ) 
            {
                const float u = i / (float)RadialSegments;
                const float x = -std::sinf( u * tau_v );
                const float z =  std::cosf( u * tau_v );
                const auto  p = float3
                {  x * Radius
                ,  y
                , -z * Radius
                };

                Mesh.m_Vertices.push_back
                ( vertex
                { .m_Position = p
                , .m_Normal   = { x, 0.0f, -z }
                , .m_Tangent  = { -z, -0.0f, -x, 1.0f }
                , .m_Texcoord = { u, onethird_v + (v * onethird_v) }
                });

                point++;

                if( i > 0 && j > 0 ) 
                {
                    Mesh.m_Indices.push_back( prevrow + i - 1 );
                    Mesh.m_Indices.push_back( thisrow + i - 1 );
                    Mesh.m_Indices.push_back( prevrow + i     );

                    Mesh.m_Indices.push_back( prevrow + i     );
                    Mesh.m_Indices.push_back( thisrow + i - 1 );
                    Mesh.m_Indices.push_back( thisrow + i     );
                }
            }

            prevrow = thisrow;
            thisrow = point;
        }

        //
        // bottom hemisphere 
        //
        thisrow = point;
        prevrow = 0;
        for( int j = 0, end_j = Rings + 1; j <= end_j; j++ ) 
        {
            const float v = j / (float)(Rings + 1) + 1;
            const float w = std::sinf( half_pi_v * v);
            const float y = std::cosf( half_pi_v * v) * Radius;

            for( int i = 0, end_i = RadialSegments; i <= end_i; ++i )
            {
                const float u2 = i/(float)RadialSegments;
                const float x  = -std::sinf( u2 * tau_v );
                const float z  =  std::cosf( u2 * tau_v );
                const auto  p  = float3
                { x * Radius * w
                , y
                , -z * Radius * w
                };

                Mesh.m_Vertices.push_back
                ( vertex
                { .m_Position = p + float3( 0.0f, -0.5f * Height + Radius, 0.0f )
                , .m_Normal   = p.Normalize()
                , .m_Tangent  = { -z, -0.0f, -x, 1.0f }
                , .m_Texcoord = { u2, twothirds_v + (v - 1.0f) * onethird_v }
                });

                point++;

                if( i > 0 && j > 0 ) 
                {
                    Mesh.m_Indices.push_back( prevrow + i - 1 );
                    Mesh.m_Indices.push_back( thisrow + i - 1 );
                    Mesh.m_Indices.push_back( prevrow + i     );

                    Mesh.m_Indices.push_back( prevrow + i     );
                    Mesh.m_Indices.push_back( thisrow + i - 1 );
                    Mesh.m_Indices.push_back( thisrow + i     );
                }
            }

            prevrow = thisrow;
            thisrow = point;
        }

        return Mesh;
    }
}





