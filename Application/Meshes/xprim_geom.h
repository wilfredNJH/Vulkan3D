#ifndef XPRIM_GEOM_H
#define XPRIM_GEOM_H
#pragma once
#include <vector>
#include <numbers>
#include <math.h>
#include <iostream>
namespace xprim_geom
{
  //  https://github.com/godotengine/godot/blob/353bb45e21fadf8da1f6fbbaaf99b8ac8acafea9/scene/resources/primitive_meshes.cpp

    constexpr auto pi_v         = std::numbers::pi_v<float>;
    constexpr auto tau_v        = 2*pi_v;
    constexpr auto half_pi_v    = pi_v/2.0f;

    struct plane
    {
        float m_X, m_Y, m_Z, m_D;
    };

    struct float3
    {
        float m_X, m_Y, m_Z;
        float  Dot      ( const float3& A ) const noexcept { return m_X*A.m_X + m_Y*A.m_Y + m_Z*A.m_Z; }
        float3 Normalize( void ) const noexcept { float d = 1.0f/std::sqrtf(Dot(*this)); return {m_X*d,m_Y*d,m_Z*d}; }
        float3 operator - (const float3& A) const noexcept { return { m_X - A.m_X, m_Y - A.m_Y, m_Z - A.m_Z }; }
        float3 operator + (const float3& A) const noexcept { return { m_X + A.m_X, m_Y + A.m_Y, m_Z + A.m_Z }; }
        float3 operator * (const float A  ) const noexcept { return { m_X * A, m_Y * A, m_Z * A }; }
        float3 operator - () const noexcept { return { -m_X, -m_Y, -m_Z }; }
    };

    struct float2
    {
        float m_X, m_Y, m_Z;
    };

    struct vertex
    {
        float3  m_Position;
        float3  m_Normal;
        plane   m_Tangent;
        float2  m_Texcoord;
    };

    struct mesh
    {
        std::vector<vertex>         m_Vertices;
        std::vector<std::uint32_t>  m_Indices;
    };
}

#include "xprim_geom_grid.h"
#include "xprim_geom_cube.h"
#include "xprim_geom_uvsphere.h"
#include "xprim_geom_cylinder.h"
#include "xprim_geom_rounded_cube.h"
#include "xprim_geom_capsule.h"
#include "xprim_geom_prism.h"
#include "xprim_geom_quad.h"
#include "xprim_geom_hemisphere.h"

#endif