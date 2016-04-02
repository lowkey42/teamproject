#include "smart_texture.hpp"


namespace mo {
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
		void triangulate_background(const std::vector<glm::vec>& points,
		                            std::vector<Sprite_vertex>& vertices) {
			// TODO: add polygon for the area inside the polygon
		}
		void triangulate_border(const std::vector<glm::vec>& points,
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
