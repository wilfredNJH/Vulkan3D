namespace xprim_geom::cube
{
    //-----------------------------------------------------------------------------
    inline
    mesh Generate( const int SubdivideX, const int SubdivideY, const int SubdivideZ, const int SubdivideW, const float3 Size ) noexcept
    {
        mesh Mesh;

        //
        // front + back
        //
        float3  StartPos = Size * -0.5f;
        int		Point	 = 0;
        float	y		 = StartPos.m_Y;
        int		thisrow  = 0;
        int		prevrow  = 0;

        for( int j = 0, end_j = SubdivideY + 1; j <= end_j; ++j )
        {
            float x = StartPos.m_X;
            for( int i = 0, end_i = SubdivideW + 1; i <= end_i; i++) 
            {
                const float u = i / (SubdivideW + 1.0f);
                const float v = j / (SubdivideY + 1.0f);

                // front
                Mesh.m_Vertices.push_back
                ( vertex
                { .m_Position = { x, -y, -StartPos.m_Z }
                , .m_Normal   = { 0.0f, 0.0f, 1.0f }
                , .m_Tangent  = { 1.0f, 0.0f, 0.0f, 1.0f }
                , .m_Texcoord = { u, v }
                });
                Point++;

                // back
                Mesh.m_Vertices.push_back
                ( vertex
                { .m_Position = { x, -y, StartPos.m_Z }
                , .m_Normal   = { 0.0f, 0.0f, -1.0f }
                , .m_Tangent  = { -1.0f, 0.0f, 0.0f, 1.0f }
                , .m_Texcoord = { 1-u, v }
                });
                Point++;

                if( i > 0 && j > 0 ) 
                {
                    const int i2 = i * 2;

                    // front
                    Mesh.m_Indices.push_back( prevrow + i2 - 2);
                    Mesh.m_Indices.push_back( thisrow + i2 - 2);
                    Mesh.m_Indices.push_back(prevrow + i2);
                    Mesh.m_Indices.push_back( prevrow + i2);
                    Mesh.m_Indices.push_back( thisrow + i2 - 2);
                    Mesh.m_Indices.push_back(thisrow + i2);

                    // back
                    Mesh.m_Indices.push_back( prevrow + i2 - 1);
                    Mesh.m_Indices.push_back(prevrow + i2 + 1);
                    Mesh.m_Indices.push_back(thisrow + i2 - 1);
                    Mesh.m_Indices.push_back( prevrow + i2 + 1);
                    Mesh.m_Indices.push_back(thisrow + i2 + 1);
                    Mesh.m_Indices.push_back(thisrow + i2 - 1);
                }

                x += Size.m_X / (SubdivideW + 1.0f);
            }

            y += Size.m_Y / (SubdivideY + 1.0f);
            prevrow = thisrow;
            thisrow = Point;
        }

        //
        // left + right
        //
        y = StartPos.m_Y;
        thisrow = Point;
        prevrow = 0;
        for( int j = 0, end_j = SubdivideY + 1 ; j <= end_j; ++j ) 
        {
            float z = StartPos.m_Z;
            for( int i = 0; i <= (SubdivideZ + 1); i++) 
            {
                const float u = i/( SubdivideZ + 1.0f);
                const float v = j/( SubdivideY + 1.0f);

                // right
                Mesh.m_Vertices.push_back
                ( vertex
                { .m_Position = { -StartPos.m_X, -y, -z }
                , .m_Normal   = { 1.0f, 0.0f, 0.0f }
                , .m_Tangent  = { 0.0f, 0.0f, -1.0f, 1.0f }
                , .m_Texcoord = { u, v }
                });
                Point++;

                // left
                Mesh.m_Vertices.push_back
                ( vertex
                { .m_Position = { StartPos.m_X, -y, -z }
                , .m_Normal   = { -1.0f, 0.0f, 0.0f }
                , .m_Tangent  = { 0.0f, 0.0f, 1.0f, 1.0f }
                , .m_Texcoord = { 1-u, v }
                });
                Point++;

                if (i > 0 && j > 0) 
                {
                    const int i2 = i * 2;

                    // right
                    Mesh.m_Indices.push_back(prevrow + i2 - 2);
                    Mesh.m_Indices.push_back(thisrow + i2 - 2);
                    Mesh.m_Indices.push_back(prevrow + i2);
                    Mesh.m_Indices.push_back(prevrow + i2);
                    Mesh.m_Indices.push_back(thisrow + i2 - 2);
                    Mesh.m_Indices.push_back(thisrow + i2);

                    // left
                    Mesh.m_Indices.push_back(prevrow + i2 - 1);
                    Mesh.m_Indices.push_back(prevrow + i2 + 1);
                    Mesh.m_Indices.push_back(thisrow + i2 - 1);
                    Mesh.m_Indices.push_back(prevrow + i2 + 1);
                    Mesh.m_Indices.push_back(thisrow + i2 + 1);
                    Mesh.m_Indices.push_back(thisrow + i2 - 1);
                }

                z += Size.m_Z / (SubdivideZ + 1.0f);
            }

            y += Size.m_Y / (SubdivideY + 1.0f);
            prevrow = thisrow;
            thisrow = Point;
        }

        // top + bottom
        float z = StartPos.m_Z;
        thisrow = Point;
        prevrow = 0;
        for( int j = 0, end_j = SubdivideZ + 1; j <= end_j; ++j )
        {
            float x = StartPos.m_X;
            for( int i = 0, end_i = SubdivideW + 1; i <= end_i; ++i ) 
            {
                const float u = i / (SubdivideW + 1.0f);
                const float v = j / (SubdivideZ + 1.0f);

                // top
                Mesh.m_Vertices.push_back
                ( vertex
                { .m_Position = { -x, -StartPos.m_Y, -z }
                , .m_Normal   = { 0.0f, 1.0f, 0.0f }
                , .m_Tangent  = { 1.0f, 0.0f, 0.0f, 1.0f }
                , .m_Texcoord = { 1 - u, 1 - v }
                });
                Point++;

                // bottom
                Mesh.m_Vertices.push_back
                ( vertex
                { .m_Position = { x, StartPos.m_Y, -z }
                , .m_Normal   = { 0.0f, -1.0f, 0.0f }
                , .m_Tangent  = { 1.0f, 0.0f, 0.0f, 1.0f }
                , .m_Texcoord = { u, v }
                });
                Point++;

                if( i > 0 && j > 0 ) 
                {
                    const int i2 = i * 2;

                    // top
                    Mesh.m_Indices.push_back(prevrow + i2 - 2);
                    Mesh.m_Indices.push_back(thisrow + i2 - 2);
                    Mesh.m_Indices.push_back(prevrow + i2);
                    Mesh.m_Indices.push_back(prevrow + i2);
                    Mesh.m_Indices.push_back(thisrow + i2 - 2);
                    Mesh.m_Indices.push_back(thisrow + i2);

                    // bottom
                    Mesh.m_Indices.push_back(prevrow + i2 - 1);
                    Mesh.m_Indices.push_back(thisrow + i2 - 1);
                    Mesh.m_Indices.push_back(prevrow + i2 + 1);
                    Mesh.m_Indices.push_back(prevrow + i2 + 1);
                    Mesh.m_Indices.push_back(thisrow + i2 - 1);
                    Mesh.m_Indices.push_back(thisrow + i2 + 1);
                }

                x += Size.m_X / (SubdivideW + 1.0f);
            }

            z += Size.m_Z / (SubdivideZ + 1.0f);
            prevrow = thisrow;
            thisrow = Point;
        }

        return Mesh;
    }
}





