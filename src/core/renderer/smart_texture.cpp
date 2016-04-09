#define GLM_SWIZZLE

#include "smart_texture.hpp"


namespace lux {
namespace renderer {

	Smart_texture::Smart_texture(Material_ptr material)
	    : _material(std::move(material)),
	      _points{{-1,-1}, {1,-1}, {1,1}, {-1,1}},
	      _dirty(true) {
	}

	void Smart_texture::move_point(std::size_t i, glm::vec2 p) {
		_points.at(i) = p;
		_dirty = true;
	}

	void Smart_texture::insert_point(std::size_t i, glm::vec2 p) {
		_points.insert(_points.begin() + i, p);
		_dirty = true;
	}

	void Smart_texture::erase_point(std::size_t i) {
		_points.erase(_points.begin() + i);
	}

	void Smart_texture::draw(glm::vec3 position, Sprite_batch& batch) {
		if(_dirty) {
			_update_vertices();
			_dirty = false;
		}

		batch.insert(position, _vertices);
	}

	namespace {
		auto cross(glm::vec2 v, glm::vec2 w) {
			return v.x*w.y - v.y*w.x;
		}
	}
	bool Smart_texture::is_inside(glm::vec2 my_pos, glm::vec2 point)const {
		point -= my_pos;

		int intersections = 0;

		for(auto i=0u; i<_points.size(); i++) {
			auto in = (i+1) % _points.size();
			auto p  = _points[i];
			auto pn = _points[in];
			auto pdiff = pn-p;

			auto s = glm::vec2{1000.f,0.f};
			auto cross_ds = cross(pdiff, s);
			auto t = cross(point-p, s/cross_ds);
			auto u = cross(point-p, pdiff/cross_ds);

			if(t>=0.f && t<=1.f && u>=0.f && u<=1.f) {
				intersections++;
			}
		}

		return (intersections % 2) == 1;
	}

	auto Smart_texture::get_point(glm::vec2 my_pos,
	                              glm::vec2 point,
	                              float point_size)const -> std::tuple<Point_location,std::size_t> {
		auto point_size2 = point_size*point_size;
		point-=my_pos;

		for(auto i=0u; i<_points.size(); i++) {
			auto p  = _points[i];

			auto dist2 = glm::length2(p-point);
			if(dist2<=point_size2) {
				return std::make_tuple(Point_location::on, i);
			}
		}

		point_size/=2.f;
		point_size2 = point_size*point_size;
		for(auto i=0u; i<_points.size(); i++) {
			auto in = (i+1) % _points.size();
			auto p  = _points[i];
			auto pn = _points[in];

			auto dist2 = glm::length2((p+(pn-p)/2.f)-point);
			if(dist2<=point_size2) {
				return std::make_tuple(Point_location::between, in);
			}
		}

		return std::make_tuple(Point_location::none, 0);
	}

	namespace {
		void triangulate_background(const std::vector<glm::vec2>& points,
		                            std::vector<Sprite_vertex>& vertices) {
			// TODO: add polygon for the area inside the polygon
		}
		void triangulate_border(const std::vector<glm::vec2>& points,
		                        std::vector<Sprite_vertex>& vertices) {
			// TODO: add rotated quad for each line segment
		}
	}
	void Smart_texture::_update_vertices() {
		_vertices.clear();

		triangulate_border(_points, _vertices);
		triangulate_background(_points, _vertices);
	}

}
}
