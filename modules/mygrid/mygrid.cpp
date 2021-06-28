/* mygrid.cpp */

#include "mygrid.h"

void MyGrid::_bind_methods() {
    BIND_ENUM_CONSTANT(solid);
    BIND_ENUM_CONSTANT(walkable);
	ClassDB::bind_method(D_METHOD("find_path", "start", "end"), &MyGrid::find_path);
	ClassDB::bind_method(D_METHOD("update_range", "start", "walk_range"), &MyGrid::update_range);

}

MyGrid::MyGrid() {
}

void MyGrid::update_range(Vector3 start, int walk_range){

	vector<IndexKey> q = vector<IndexKey>();

	IndexKey key;
	key.x = start.x;
	key.y = start.y;
	key.z = start.z;

	q.push_back(key);

	in_range[key] = MyGrid::DistanceTo();
	in_range[key].distance = 0;

	while (!q.empty()){
		key = q.back();
		q.pop_back();
        
        // Check surrounding cells
        IndexKey rkey = key;
        rkey.x++;
        add_in_range(key, rkey, q, walk_range);
        IndexKey lkey = key;
        lkey.x--;
        add_in_range(key, lkey, q, walk_range);
        IndexKey fkey = key;
        fkey.z++;
        add_in_range(key, fkey, q, walk_range);
        IndexKey bkey = key;
        bkey.z--;
        add_in_range(key, bkey, q, walk_range);

	}

	for (Map<IndexKey, DistanceTo>::Element * it = in_range.front(); true; it = it->next()) {
		IndexKey ik = it->key();
		set_cell_item(ik.x, ik.y, ik.z, 1);
		if (it == in_range.back()){
			break;
		}
	}
}

void MyGrid::add_in_range(IndexKey source, IndexKey target, vector<IndexKey> &q, float walk_range){
	if (is_walkable(target)) {
		float new_dist = in_range[source].distance + 1.0;
		if (new_dist < walk_range) {
			if (in_range.has(target)) {
				if (new_dist < in_range[target].distance) {
					in_range[target].distance = new_dist;
					in_range[target].source = source;
					q.push_back(target);
				}
			} else {
				in_range[target] = MyGrid::DistanceTo();
				in_range[target].distance = new_dist;
				in_range[target].source = source;
				q.push_back(target);
			}
		}
	}
};

bool MyGrid::is_walkable(IndexKey key){
	int i = get_cell_item(key.x, key.y, key.z);
	if (mesh_library->has_item(i)) {
		GridcellType cell_type = mesh_library->get_item_celltype(i);
		if (cell_type == walkable){
			key.y -= 1;
			return is_solid(key);
		}
	} else {
		key.y -= 1;
		return is_solid(key);
	}
	return false;
}

bool MyGrid::is_solid(IndexKey key) {
	int i = get_cell_item(key.x, key.y, key.z);
	if (mesh_library->has_item(i)) {
		GridcellType cell_type = mesh_library->get_item_celltype(i);
		return (cell_type == solid);
	}
	return false;
}

Vector<Vector3> MyGrid::find_path(Vector3 start, Vector3 end) {

    Vector<Vector3> path;

    return path;
}
