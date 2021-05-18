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

	Map<IndexKey, DistanceTo> in_range;

protected:
	static void _bind_methods();

public:
	MyGrid();
	bool is_solid(IndexKey key);
	bool is_walkable(IndexKey ik);
	void add_in_range(IndexKey source, IndexKey target, vector<IndexKey> &q, float walk_range);
	void update_range(Vector3 start, int walk_range);
	Vector<Vector3> find_path(Vector3 start, Vector3 end);

};



#endif // MYGRID_H