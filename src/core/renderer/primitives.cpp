#include "primitives.hpp"

namespace mo {
namespace renderer {

	Vertex_layout simple_vertex_layout {
		Vertex_layout::Mode::triangles,
		vertex("position", &Simple_vertex::xy),
		vertex("uv", &Simple_vertex::uv)
	};

}
}
