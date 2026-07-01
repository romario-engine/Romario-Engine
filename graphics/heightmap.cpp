#include "graphics/heightmap.h"

heightmap::heightmap()
{
	map = nullptr;
}

heightmap::~heightmap()
{
	if(map != nullptr) delete[] map;
}

void heightmap::resize(uint64 width_value, uint64 length_value)
{
	width = width_value;
	length = length_value;
	if(map != nullptr) delete[] map;
	map = new heightmap_unit[width * length];
}

void heightmap::generate_model(ID3D11Device *device)
{
	map_model.release_render_resources();
	map_model.vertices.clear();
	map_model.indices.clear();
	map_model.vertices.ensure_capacity((width + 1) * (length + 1));
	world_vertex v;
	v.normal = vector<float32, 3>(0.0f, 0.0f, 0.0f);
	v.point.z = start_z;
	vector<float32, 2> uv(0.0f, float32(length + 1));
	uint64 i1, j1;
	for(uint64 i = 0; i < length + 1; i++)
	{
		v.point.x = start_x;
		uv.x = 0.0f;
		for(uint64 j = 0; j < width + 1; j++)
		{
			if(i < length) i1 = i;
			else i1 = length - 1;
			if(j < width) j1 = j;
			else j1 = width - 1;
			v.point.y = map[i1 * width + j1].height;
			v.texture = uv;
			map_model.vertices.push_back(v);
			v.point.x += unit_size;
			uv.x += 1.0f;
		}
		v.point.z += unit_size;
		uv.y -= 1.0f;
	}
	map_model.indices.ensure_capacity(width * length * 6);
	vector<float32, 3> v1, v2, v3;
	for(uint64 i = 0; i < length; i++)
	{
		for(uint64 j = 0; j < width; j++)
		{
			map_model.indices.push_back(uint32(i * (width + 1) + j));
			map_model.indices.push_back(uint32((i + 1) * (width + 1) + j));
			map_model.indices.push_back(uint32(i * (width + 1) + j + 1));
			v1 = map_model.vertices[i * (width + 1) + j + 1].point - map_model.vertices[i * (width + 1) + j].point;
			v2 = map_model.vertices[(i + 1) * (width + 1) + j].point - map_model.vertices[i * (width + 1) + j].point;
			v3 = vector_cross(v1, v2);
			map_model.vertices[i * (width + 1) + j].normal += v3;
			map_model.vertices[(i + 1) * (width + 1) + j].normal += v3;
			map_model.vertices[i * (width + 1) + j + 1].normal += v3;
			map_model.indices.push_back(uint32(i * (width + 1) + j + 1));
			map_model.indices.push_back(uint32((i + 1) * (width + 1) + j));
			map_model.indices.push_back(uint32((i + 1) * (width + 1) + j + 1));
			v1 = map_model.vertices[(i + 1) * (width + 1) + j + 1].point - map_model.vertices[i * (width + 1) + j + 1].point;
			v2 = map_model.vertices[(i + 1) * (width + 1) + j].point - map_model.vertices[i * (width + 1) + j + 1].point;
			v3 = vector_cross(v1, v2);
			map_model.vertices[i * (width + 1) + j + 1].normal += v3;
			map_model.vertices[(i + 1) * (width + 1) + j].normal += v3;
			map_model.vertices[(i + 1) * (width + 1) + j + 1].normal += v3;
		}
	}
	for(uint64 i = 0; i < map_model.vertices.size; i++)
		map_model.vertices[i].normal = vector_normal(map_model.vertices[i].normal);
	map_model.initialize_render_resources(device);
}
