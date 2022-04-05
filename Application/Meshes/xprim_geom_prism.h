namespace xprim_geom::prism
{
    //-----------------------------------------------------------------------------
    inline
    mesh Generate( int SubdivideY, int SubdivideX, int SubdivideZ, float LeftToRight, float3 Size ) noexcept
    {
        constexpr float onethird_v	= 1.0f / 3.0f;
        constexpr float twothirds_v = 2.0f / 3.0f;

        mesh Mesh;

        //
        // front + back 
        //
        const auto  StartPos    = Size * -0.5;
        float       y           = StartPos.m_Y;
        int         Point       = 0;
        int         thisrow     = 0;
        int         prevrow     = 0;
        for( int j = 0, end_j = SubdivideY + 1; j <= end_j; j++)
        {
            const float Scale           = (y - StartPos.m_Y) / Size.m_Y;
            const float ScaledSizeX     = Size.m_X * Scale;
            const float StartX          = StartPos.m_X + (1.0f - Scale) * Size.m_X * LeftToRight;
            const float OffsetFront     = (1.0f - Scale) * onethird_v * LeftToRight;
            const float OffsetBack      = (1.0f - Scale) * onethird_v * (1.0f - LeftToRight);
            float       x               = 0.0f;

            for( int i = 0, end_i = SubdivideX + 1; i <= end_i; i++ ) 
            {
                const float u = i / (3.0f * (SubdivideX  + 1.0f)) * Scale;
                const float v = j / (2.0f * (SubdivideY + 1.0f));

                //
                // front 
                //
                Mesh.m_Vertices.push_back
                ( vertex
                { .m_Position = { StartX + x, -y, -StartPos.m_Z }	// double negative on the Z!
                , .m_Normal   = { 0.0f, 0.0f, 1.0f }
                , .m_Tangent  = { 1.0f, 0.0f, 0.0f, 1.0f }
                , .m_Texcoord = { OffsetFront + u, v }
                });
                Point++;

                //
                // back 
                //
                Mesh.m_Vertices.push_back
                ( vertex
                { .m_Position = { StartX + ScaledSizeX - x, -y, StartPos.m_Z }	// double negative on the Z!
                , .m_Normal   = { 0.0f, 0.0f, -1.0f }
                , .m_Tangent  = { -1.0f, 0.0f, 0.0f, 1.0f }
                , .m_Texcoord = { twothirds_v + OffsetBack + u, v }
                });
                Point++;

                if( i > 0 && j == 1 ) 
                {
                    const int i2 = i * 2;

                    // front
                    Mesh.m_Indices.push_back(prevrow + i2);
                    Mesh.m_Indices.push_back(thisrow + i2 - 2);
                    Mesh.m_Indices.push_back(thisrow + i2);

                    // back
                    Mesh.m_Indices.push_back(prevrow + i2 + 1);
                    Mesh.m_Indices.push_back(thisrow + i2 - 1);
                    Mesh.m_Indices.push_back(thisrow + i2 + 1);
                }
                else if (i > 0 && j > 0) 
                {
                    const int i2 = i * 2;

                    // front 
                    Mesh.m_Indices.push_back(prevrow + i2 - 2);
                    Mesh.m_Indices.push_back(thisrow + i2 - 2);
                    Mesh.m_Indices.push_back(prevrow + i2);
                    Mesh.m_Indices.push_back(prevrow + i2);
                    Mesh.m_Indices.push_back(thisrow + i2 - 2);
                    Mesh.m_Indices.push_back(thisrow + i2);

                    // back 
                    Mesh.m_Indices.push_back(prevrow + i2 - 1);
                    Mesh.m_Indices.push_back(thisrow + i2 - 1);
                    Mesh.m_Indices.push_back(prevrow + i2 + 1);
                    Mesh.m_Indices.push_back(prevrow + i2 + 1);
                    Mesh.m_Indices.push_back(thisrow + i2 - 1);
                    Mesh.m_Indices.push_back(thisrow + i2 + 1);
                }

                x += Scale * Size.m_X / (SubdivideX + 1.0f);
            }

            y += Size.m_Y / (SubdivideY + 1.0f);
            prevrow = thisrow;
            thisrow = Point;
        }

        //
        // Left + Right
        //
        const auto NormalLeft  = float3(-Size.m_Y, Size.m_X * LeftToRight, 0.0f).Normalize();
        const auto NormalRight = float3(Size.m_Y, Size.m_X * (1.0f - LeftToRight), 0.0f).Normalize();

        y       = StartPos.m_Y;
        thisrow = Point;
        prevrow = 0;
        for( int j = 0, end_j = SubdivideY + 1; j <= end_j; j++ ) 
        {
            const float Scale   = (y - StartPos.m_Y) / Size.m_Y;
            const float Left    = StartPos.m_X + (Size.m_X * (1.0f - Scale) * LeftToRight);
            const float Right   = Left + (Size.m_X * Scale);
            float       z       = StartPos.m_Z;

            for( int i = 0, end_i = SubdivideZ + 1; i <= end_i; i++) 
            {
                const float u = i / (3.0f * (SubdivideZ + 1.0f));
                const float v = j / (2.0f * (SubdivideY + 1.0f));

                //
                // Right 
                //
                Mesh.m_Vertices.push_back
                ( vertex
                { .m_Position = { Right, -y, -z }
                , .m_Normal   = { NormalRight }
                , .m_Tangent  = { 0.0f, 0.0f, -1.0f, 1.0f }
                , .m_Texcoord = { onethird_v + u, v }
                });
                Point++;

                //
                // Left 
                //
                Mesh.m_Vertices.push_back
                ( vertex
                { .m_Position = { Left, -y, z }
                , .m_Normal   = { NormalLeft }
                , .m_Tangent  = { 0.0f, 0.0f, 1.0f, 1.0f }
                , .m_Texcoord = { u, 0.5f + v }
                });
                Point++;

                if( i > 0 && j > 0 ) 
                {
                    const int i2 = i * 2;

                    // Right 
                    Mesh.m_Indices.push_back(prevrow + i2 - 2);
                    Mesh.m_Indices.push_back(thisrow + i2 - 2);
                    Mesh.m_Indices.push_back(prevrow + i2);
                    Mesh.m_Indices.push_back(prevrow + i2);
                    Mesh.m_Indices.push_back(thisrow + i2 - 2);
                    Mesh.m_Indices.push_back(thisrow + i2);

                    // Left 
                    Mesh.m_Indices.push_back(prevrow + i2 - 1);
                    Mesh.m_Indices.push_back(thisrow + i2 - 1);
                    Mesh.m_Indices.push_back(prevrow + i2 + 1);
                    Mesh.m_Indices.push_back(prevrow + i2 + 1);
                    Mesh.m_Indices.push_back(thisrow + i2 - 1);
                    Mesh.m_Indices.push_back(thisrow + i2 + 1);
                }

                z += Size.m_Z / (SubdivideZ + 1.0f);
            }

            y += Size.m_Y / (SubdivideY + 1.0f);
            prevrow = thisrow;
            thisrow = Point;
        }

        //
        // bottom 
        //
        float z = StartPos.m_Z;
        thisrow = Point;
        prevrow = 0;

        for( int j = 0, end_j = SubdivideZ + 1; j <= end_j; ++j ) 
        {
            float x = StartPos.m_X;

            for( int i = 0, end_i = SubdivideX + 1; i <= end_i; ++i )
            {
                const float u = i / (3.0f * (SubdivideX + 1.0f));
                const float v = j / (2.0f * (SubdivideZ + 1.0f));

                //
                // bottom 
                //
                Mesh.m_Vertices.push_back
                ( vertex
                { .m_Position = { x, StartPos.m_Y, -z }
                , .m_Normal   = { 0.0f, -1.0f, 0.0f }
                , .m_Tangent  = { 1.0f, 0.0f, 0.0f, 1.0f }
                , .m_Texcoord = { twothirds_v + u, 0.5f + v }
                });
                Point++;

                if (i > 0 && j > 0) 
                {
                    // bottom 
                    Mesh.m_Indices.push_back(prevrow + i - 1);
                    Mesh.m_Indices.push_back(thisrow + i - 1);
                    Mesh.m_Indices.push_back(prevrow + i);
                    Mesh.m_Indices.push_back(prevrow + i);
                    Mesh.m_Indices.push_back(thisrow + i - 1);
                    Mesh.m_Indices.push_back(thisrow + i);
                }

                x += Size.m_X / (SubdivideX + 1.0f);
            }

            z += Size.m_Z / (SubdivideZ + 1.0f);
            prevrow = thisrow;
            thisrow = Point;
        }

        return Mesh;
    }
}

