#include "polygon_separator.hpp"

#include <core/utils/maybe.hpp>

#include <deque>


namespace lux {
namespace sys {
namespace physics {

	namespace {
		auto toB2Vec2(glm::vec2 v) {
			return b2Vec2{v.x, v.y};
		}
	}

	void create_polygons(const std::vector<glm::vec2> points,
	                     b2Body& body, b2FixtureDef fixture) {

		b2EdgeShape shape;
		shape.m_hasVertex0 = true;
		shape.m_hasVertex3 = true;

		for(auto i=0u; i<points.size(); ++i) {
			shape.m_vertex0 = toB2Vec2(points[i>0 ? i-1 : points.size()-1]);
			shape.m_vertex1 = toB2Vec2(points[i]);
			shape.m_vertex2 = toB2Vec2(points[(i+1) % points.size()]);
			shape.m_vertex3 = toB2Vec2(points[(i+2) % points.size()]);

			fixture.shape = &shape;
			body.CreateFixture(&fixture);
		}
	}

}
}
}
