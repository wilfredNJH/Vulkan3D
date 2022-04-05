#include <span>

namespace xprim_geom::rounded_cube
{
    //-----------------------------------------------------------------------------
    inline
    mesh Generate( int SizeX, int SizeY, int SizeZ, float Roundness ) noexcept
    {
        mesh Mesh;

        //
        // Generate Verts
        //
        {
            const int CornerVertices = 8;
            const int EdgeVertices   = (SizeX + SizeY + SizeZ - 3) * 4;
            const int FaceVertices   = ( (SizeX - 1) * (SizeY - 1)
                                       + (SizeX - 1) * (SizeZ - 1)
                                       + (SizeY - 1) * (SizeZ - 1) ) * 2;

            Mesh.m_Vertices = std::vector<vertex>( CornerVertices + EdgeVertices + FaceVertices );

            auto SetVertex = [&]( int i, int x, int y, int z ) 
            {
                auto Inner = Mesh.m_Vertices[i].m_Position = float3{ static_cast<float>(x), static_cast<float>(y), static_cast<float>(z) };

                if( x < Roundness ) 
                {
                    Inner.m_X = Roundness;
                }
                else if ( x > SizeX - Roundness)
                {
                    Inner.m_X = SizeX - Roundness;
                }
                if (y < Roundness)
                {
                    Inner.m_Y = Roundness;
                }
                else if (y > SizeY - Roundness)
                {
                    Inner.m_Y = SizeY - Roundness;
                }
                if (z < Roundness)
                {
                    Inner.m_Z = Roundness;
                }
                else if (z > SizeZ - Roundness)
                {
                    Inner.m_Z = SizeZ - Roundness;
                }

                Mesh.m_Vertices[i].m_Normal   = (Mesh.m_Vertices[i].m_Position - Inner).Normalize();
                Mesh.m_Vertices[i].m_Position = Inner + Mesh.m_Vertices[i].m_Normal * Roundness;
                //Mesh.m_Vertices[i].m_Texcoord = cubeUV[i] = new Color32((byte)x, (byte)y, (byte)z, 0);
                // Deal with these uvs as colors?
            };

            int v = 0;
            for (int y = 0; y <= SizeY; y++)
            {
                for (int x = 0; x <= SizeX; x++) 
                {
                    SetVertex(v++, x, y, 0);
                }
                for (int z = 1; z <= SizeZ; z++)
                {
                    SetVertex(v++, SizeX, y, z);
                }
                for (int x = SizeX - 1; x >= 0; x--)
                {
                    SetVertex(v++, x, y, SizeZ);
                }
                for (int z = SizeZ - 1; z > 0; z--)
                {
                    SetVertex(v++, 0, y, z);
                }
            }

            for (int z = 1; z < SizeZ; z++)
            {
                for (int x = 1; x < SizeX; x++)
                {
                    SetVertex(v++, x, SizeY, z);
                }
            }

            for (int z = 1; z < SizeZ; z++) 
            {
                for (int x = 1; x < SizeX; x++)
                {
                    SetVertex(v++, x, 0, z);
                }
            }

        }

        //
        // Set faces
        //
        auto trianglesZ = std::vector<int>(SizeX * SizeY * 12);
        auto trianglesX = std::vector<int>(SizeY * SizeZ * 12);
        auto trianglesY = std::vector<int>(SizeX * SizeZ * 12);
        const int  Ring = (SizeX + SizeZ) * 2;
        int        tZ   = 0, tX = 0, v = 0;


        auto SetQuad = [&](std::span<int> Span, int i, int v00, int v10, int v01, int v11) -> int
        {
            //assert( v00 < Mesh.m_Vertices.size() );
            //assert(v10 < Mesh.m_Vertices.size());
            //assert(v01 < Mesh.m_Vertices.size());
            //assert(v11 < Mesh.m_Vertices.size());
            //assert( v00 >= 0);
            //assert(v10 >= 0);
            //assert(v01 >= 0);
            //assert(v11 >= 0);


            Span[i] = v00;
            Span[i + 1] = Span[i + 4] = v01;
            Span[i + 2] = Span[i + 3] = v10;
            Span[i + 5] = v11;
            return i + 6;
        };

        for (int y = 0; y < SizeY; y++, v++)
        {
            for (int q = 0; q < SizeX; q++, v++)
            {
                tZ = SetQuad(trianglesZ, tZ, v, v + 1, v + Ring, v + Ring + 1);
            }
            for (int q = 0; q < SizeZ; q++, v++) 
            {
                tX = SetQuad(trianglesX, tX, v, v + 1, v + Ring, v + Ring + 1);
            }
            for (int q = 0; q < SizeX; q++, v++) 
            {
                tZ = SetQuad(trianglesZ, tZ, v, v + 1, v + Ring, v + Ring + 1);
            }
            for (int q = 0; q < SizeZ - 1; q++, v++) 
            {
                tX = SetQuad(trianglesX, tX, v, v + 1, v + Ring, v + Ring + 1);
            }
            tX = SetQuad(trianglesX, tX, v, v - Ring + 1, v + Ring, v + 1);
        }

        auto CreateTopFace = [&]( std::span<int> Triangles, int t = 0 ) -> int
        {
            int v = Ring * SizeY;
            for (int x = 0; x < SizeX - 1; x++, v++)
            {
                t = SetQuad(Triangles, t, v, v + 1, v + Ring - 1, v + Ring);
            }
            t = SetQuad(Triangles, t, v, v + 1, v + Ring - 1, v + 2);

            int vMin = Ring * (SizeY + 1) - 1;
            int vMid = vMin + 1;
            int vMax = v + 2;
            for( int z = 1;  z < SizeZ - 1; z++, vMin--, vMid++, vMax++ )
            {
                t = SetQuad(Triangles, t, vMin, vMid, vMin - 1, vMid + SizeX - 1);
                for( int x = 1; x < SizeX - 1; x++, vMid++ )
                {
                    t = SetQuad
                    (Triangles
                    , t
                    , vMid
                    , vMid + 1
                    , vMid + SizeX - 1
                    , vMid + SizeX
                    );
                }	

                t = SetQuad(Triangles, t, vMid, vMax, vMid + SizeX - 1, vMax + 1);
            }

            int vTop = vMin - 2;
            t = SetQuad(Triangles, t, vMin, vMid, vTop + 1, vTop);
            for (int x = 1; x < SizeX - 1; x++, vTop--, vMid++) {
                t = SetQuad(Triangles, t, vMid, vMid + 1, vTop, vTop - 1);
            }
            t = SetQuad(Triangles, t, vMid, vTop - 2, vTop, vTop - 1);

            return t;
        };

        auto CreateBottomFace = [&]( std::span<int> Triangles, int t ) -> int
        {
            int v = 1;
            int vMid = static_cast<int>(Mesh.m_Vertices.size()) - (SizeX - 1) * (SizeZ - 1);
            t = SetQuad(Triangles, t, Ring - 1, vMid, 0, 1);
            for (int x = 1; x < SizeX - 1; x++, v++, vMid++) {
                t = SetQuad(Triangles, t, vMid, vMid + 1, v, v + 1);
            }
            t = SetQuad(Triangles, t, vMid, v + 2, v, v + 1);

            int vMin = Ring - 2;
            vMid -= SizeX - 2;
            int vMax = v + 2;

            for( int z = 1; z < SizeZ - 1; z++, vMin--, vMid++, vMax++ ) 
            {
                t = SetQuad(Triangles, t, vMin, vMid + SizeX - 1, vMin + 1, vMid);
                for( int x = 1; x < SizeX - 1; x++, vMid++ ) 
                {
                    t = SetQuad
                    ( Triangles
                    , t
                    , vMid + SizeX - 1
                    , vMid + SizeX
                    , vMid, vMid + 1
                    );
                }
                t = SetQuad(Triangles, t, vMid + SizeX - 1, vMax + 1, vMid, vMax);
            }

            int vTop = vMin - 1;
            t = SetQuad(Triangles, t, vTop + 1, vTop, vTop + 2, vMid);

            for( int x = 1; x < SizeX - 1; x++, vTop--, vMid++ ) 
            {
                t = SetQuad(Triangles, t, vTop, vTop - 1, vMid, vMid + 1);
            }
            t = SetQuad( Triangles, t, vTop, vTop - 1, vMid, vTop - 2 );

            return t;
        };

        CreateBottomFace(trianglesY, CreateTopFace(trianglesY) );

        //
        // lets move all the indices together
        //
        Mesh.m_Indices.resize(trianglesZ.size() + trianglesX.size() + trianglesY.size() );

        std::memcpy( &Mesh.m_Indices[0], trianglesZ.data(), trianglesZ.size() * sizeof(std::uint32_t) );
        auto c = trianglesZ.size();

        std::memcpy(&Mesh.m_Indices[c], trianglesX.data(), trianglesX.size() * sizeof(std::uint32_t));
        c += trianglesX.size();

        std::memcpy(&Mesh.m_Indices[c], trianglesY.data(), trianglesY.size() * sizeof(std::uint32_t));

        return Mesh;
    }
}



