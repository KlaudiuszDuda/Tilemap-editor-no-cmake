#pragma once

#include <glm/glm.hpp>
#include "Viewport.h"

struct Plane
{
	glm::vec3 normal = { 0.f, 1.f, 0.f }; // unit vector
	float     distance = 0.f;        // Distance with origin

	Plane() = default;

	Plane(const glm::vec3& p1, const glm::vec3& norm)
		: normal(glm::normalize(norm)),
		distance(glm::dot(normal, p1))
	{
	}

	inline float getSignedDistanceToPlane(const glm::vec3& point) const
	{
		return glm::dot(normal, point) - distance;
	}
};

struct Frustum
{
	Plane topFace;
	Plane bottomFace;

	Plane rightFace;
	Plane leftFace;

	Plane farFace;
	Plane nearFace;
};

struct AABB
{
	glm::vec3 m_Center{ 0.f, 0.f, 0.f };
	glm::vec3 m_Extents{ 0.f, 0.f, 0.f };

	inline AABB(glm::vec3 center, glm::vec3 extents)
	{
		m_Center = center;
		m_Extents = extents;
	}

	bool isOnOrForwardPlane(const Plane& plane)
	{
		// Compute the projection interval radius of b onto L(t) = b.c + t * p.n
		const float r = m_Extents.x * std::abs(plane.normal.x) + m_Extents.y * std::abs(plane.normal.y) +
			m_Extents.z * std::abs(plane.normal.z);

		return -r <= plane.getSignedDistanceToPlane(m_Center);
	}
};

inline AABB setAABB(const glm::vec3& min, const glm::vec3& max)
{
	glm::vec3 center = { (max + min) * 0.5f };
	return AABB(center, { max.x - center.x, max.y - center.y, max.z - center.z });
}

inline bool isAABBOnFrustum(AABB aabb, const Frustum& camFrustum)
{
	return (aabb.isOnOrForwardPlane(camFrustum.leftFace) &&
		aabb.isOnOrForwardPlane(camFrustum.rightFace) &&
		aabb.isOnOrForwardPlane(camFrustum.topFace) &&
		aabb.isOnOrForwardPlane(camFrustum.bottomFace) &&
		aabb.isOnOrForwardPlane(camFrustum.nearFace) &&
		aabb.isOnOrForwardPlane(camFrustum.farFace));
};

inline Frustum createFrustumFromCamera(const Viewport& cam, float aspect, float fovY, float zNear, float zFar)
{
	Frustum     frustum;
	const float halfVSide = zFar * tanf(fovY * .5f);
	const float halfHSide = halfVSide * aspect;
	const glm::vec3 frontMultFar = zFar * cam.Front;

	frustum.nearFace = { cam.Position + zNear * cam.Front, cam.Front };
	frustum.farFace = { cam.Position + frontMultFar, -cam.Front };
	frustum.rightFace = { cam.Position, glm::cross(frontMultFar - cam.Right * halfHSide, cam.Up) };
	frustum.leftFace = { cam.Position, glm::cross(cam.Up, frontMultFar + cam.Right * halfHSide) };
	frustum.topFace = { cam.Position, glm::cross(cam.Right, frontMultFar - cam.Up * halfVSide) };
	frustum.bottomFace = { cam.Position, glm::cross(frontMultFar + cam.Up * halfVSide, cam.Right) };
	return frustum;
}