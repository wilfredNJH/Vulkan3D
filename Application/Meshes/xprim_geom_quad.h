#include <array>

namespace xprim_geom::quad
{
    //-----------------------------------------------------------------------------
    inline
    mesh Generate( float2 Size, float3 CenterOffset = {0,0,0}) noexcept
    {
        mesh Mesh;

        const auto HalfSize = float2(Size.m_X / 2.0f, Size.m_Y / 2.0f);

        const auto QuadFaces = std::array 
        { float3(-HalfSize.m_X, -HalfSize.m_Y, 0) + CenterOffset
        , float3(-HalfSize.m_X,  HalfSize.m_Y, 0) + CenterOffset
        , float3( HalfSize.m_X,  HalfSize.m_Y, 0) + CenterOffset
        , float3( HalfSize.m_X, -HalfSize.m_Y, 0) + CenterOffset
        };

        static constexpr auto uvs = std::array
        {	float2(0, 1)
        ,	float2(0, 0)
        ,	float2(1, 0)
        ,	float2(1, 1)
        };

        static constexpr auto indices_v = std::array
        { 0, 2, 1
        , 0, 3, 2
        };

        Mesh.m_Indices.resize(indices_v.size());
        Mesh.m_Vertices.reserve(uvs.size());

        for( int i = 0; i < 4; i++ ) 
        {
            Mesh.m_Vertices.push_back
            ( vertex
            { .m_Position = QuadFaces[i]
            , .m_Normal   = { 0, 0, 1 }
            , .m_Tangent  = { 1.0f, 0.0f, 0.0f, 1.0f }
            , .m_Texcoord = uvs[i]
            });
        }

        // copy indices
        std::memcpy( Mesh.m_Indices.data(), indices_v.data(), indices_v.size() * sizeof(int) );

        return Mesh;
    }
}

