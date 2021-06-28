/* mygrid.h */

#ifndef MYGRID_H
#define MYGRID_H

#include "core/reference.h"
#include "modules/gridmap/grid_map.h"
#include "scene/resources/mesh_library.h"
#include <iostream>
#include <vector>

using namespace std;


class MyGrid : public GridMap {
	GDCLASS(MyGrid, GridMap);

	struct DistanceTo {
		IndexKey source;
		float distance;
	};
	Ref<MeshLibrary> basics_library;
	Ref<Material> basics_material;
	Map<IndexKey, DistanceTo> in_range;
	IndexKey start_ik;
	float range = 4.0;
protected:
	static void _bind_methods();

public:
	MyGrid();
	bool is_solid(IndexKey key);
	bool is_walkable(IndexKey ik);
	void add_in_range(IndexKey source, IndexKey target, vector<IndexKey> &q, float walk_range);
	void update_range(Vector3 start, int walk_range);
	Vector<Vector3> find_path(Vector3 start, Vector3 end);

	void set_basics_library(const Ref<MeshLibrary> &p_mesh_library);
	Ref<MeshLibrary> get_basics_library() const;

	void set_basics_mat(const Ref<Material> &p_mat);
	Ref<Material> get_basics_mat() const;

	void update_autotile();


};

bool is_equal(GridMap::IndexKey ik1, GridMap::IndexKey ik2);

#endif // MYGRID_H