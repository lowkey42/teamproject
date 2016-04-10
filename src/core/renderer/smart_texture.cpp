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
		_dirty = true;
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
		using namespace glm;

		auto is_convex(vec2 a, vec2 b, vec2 c) {
			return ( ( a.x * ( c.y - b.y ) ) + ( b.x * ( a.y - c.y ) ) + ( c.x * ( b.y - a.y ) ) ) < 0.f;
		}
		auto sign(vec2 p1, vec2 p2, vec2 p3){
			return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
		}

		auto is_in_triangle(vec2 pt, vec2 v1, vec2 v2, vec2 v3){
			auto b1 = sign(pt, v1, v2) < 0.0f;
			auto b2 = sign(pt, v2, v3) < 0.0f;
			auto b3 = sign(pt, v3, v1) < 0.0f;

			return b1 == b2 && b2 == b3;
		}
		auto is_ear(std::size_t a, std::size_t b, std::size_t c, const std::vector<vec2>& points) {
			auto prev = points[a];
			auto curr = points[b];
			auto next = points[c];

			for(auto i=0u; i<points.size(); i++) {
				if(i!=a && i!=b && i!=c && is_in_triangle(points[i], prev, curr, next)) {
					return false;
				}
			}

			return true;
		}

		void triangulate_background(std::vector<glm::vec2> points,
		                            std::vector<Sprite_vertex>& vertices,
		                            bool shadowcaster, const renderer::Material& mat) {

			auto add_vertex = [&](auto v) {
				auto uv_clip = vec4{0.f, 0.25f, 0.75f, 1.f};
				vertices.emplace_back(vec3(v,0.f), v, uv_clip, 0.f, shadowcaster ? 1.f : 0.f, &mat);
			};

			int triangles_produced = 0;
			do {
				triangles_produced = 0;

				for(auto i=0u; i<points.size() && points.size()>3; i++) {
					auto pi = i>0 ? i-1 : points.size()-1;
					auto ni = (i+1) % points.size();

					auto prev = points[pi];
					auto curr = points[i];
					auto next = points[ni];

					if(is_convex(prev, curr, next) && is_ear(pi,i,ni,points)) {
						points.erase(points.begin()+i);
						triangles_produced++;

						// create triangle (prev, curr, next)
						add_vertex(prev);
						add_vertex(curr);
						add_vertex(next);
					}
				}

			} while(points.size()>3 && triangles_produced>0);

			if(points.size()==3) {
				add_vertex(points[0]);
				add_vertex(points[1]);
				add_vertex(points[2]);

			} else {
				INFO("Polygon is not valid. "<<points.size()<<" vertices left");
				vertices.clear();
			}
		}
		void triangulate_border(const std::vector<glm::vec2>& points,
		                        std::vector<Sprite_vertex>& vertices) {
			// TODO: add rotated quad for each line segment
		}
	}
	void Smart_texture::_update_vertices() {
		_vertices.clear();

		triangulate_border(_points, _vertices);
		triangulate_background(_points, _vertices, _shadowcaster, *_material);
	}

}
}
