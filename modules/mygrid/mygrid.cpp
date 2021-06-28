/* mygrid.cpp */

#include "mygrid.h"

void MyGrid::_bind_methods() {
    BIND_ENUM_CONSTANT(solid);
    BIND_ENUM_CONSTANT(walkable);
	ClassDB::bind_method(D_METHOD("find_path", "start", "end"), &MyGrid::find_path);
	ClassDB::bind_method(D_METHOD("update_range", "start", "walk_range"), &MyGrid::update_range);
	ClassDB::bind_method(D_METHOD("update_autotile"), &MyGrid::update_autotile);

	ClassDB::bind_method(D_METHOD("set_basics_library", "mesh_library"), &MyGrid::set_basics_library);
	ClassDB::bind_method(D_METHOD("get_basics_library"), &MyGrid::get_basics_library);

	ClassDB::bind_method(D_METHOD("set_basics_mat", "mesh_library"), &MyGrid::set_basics_mat);
	ClassDB::bind_method(D_METHOD("get_basics_mat"), &MyGrid::get_basics_mat);

	
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "basics_meshlib", PROPERTY_HINT_RESOURCE_TYPE, "MeshLibrary"), "set_basics_library", "get_basics_library");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "basics_material", PROPERTY_HINT_RESOURCE_TYPE, "Material"), "set_basics_mat", "get_basics_mat");

}

MyGrid::MyGrid() {
}

void MyGrid::set_basics_library(const Ref<MeshLibrary> &p_mesh_library){

	if (!basics_library.is_null())
		basics_library->unregister_owner(this);
	basics_library = p_mesh_library;
	if (!basics_library.is_null())
		basics_library->register_owner(this);

	basics_library = p_mesh_library;
}

Ref<MeshLibrary> MyGrid::get_basics_library() const{
	return basics_library;
}

void MyGrid::set_basics_mat(const Ref<Material> &p_mat){
	if (!basics_material.is_null())
		basics_material->unregister_owner(this);
	basics_material = p_mat;
	if (!basics_material.is_null())
		basics_material->register_owner(this);

	basics_material = p_mat;
}

Ref<Material> MyGrid::get_basics_mat() const {
	return basics_material;
}

void MyGrid::update_range(Vector3 start, int walk_range){

	vector<IndexKey> q = vector<IndexKey>();

	IndexKey key;
	key.x = start.x;
	key.y = start.y;
	key.z = start.z;
	start_ik = key;

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
	/*
	for (Map<IndexKey, DistanceTo>::Element * it = in_range.front(); true; it = it->next()) {
		IndexKey ik = it->key();
		set_cell_item(ik.x, ik.y, ik.z, 1);
		if (it == in_range.back()){
			break;
		}
	}*/
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

	IndexKey s_ik;
	s_ik.x = start.x;
	s_ik.y = start.y;
	s_ik.z = start.z;

	if (!is_equal(start_ik, s_ik)){
		MyGrid::update_range(start, range);
	}

	IndexKey e_ik;
	e_ik.x = end.x;
	e_ik.y = end.y;
	e_ik.z = end.z;

	cout << "start";
	while (!is_equal(e_ik, s_ik)){
		path.push_back(Vector3(e_ik.x, e_ik.y, e_ik.z));

		if (in_range.has(e_ik)){
			if (is_equal(e_ik, in_range[e_ik].source)){
				cout << "aha";
				return Vector<Vector3>();
			}
			e_ik = in_range[e_ik].source;
		}else{
			cout << "aha";
			return Vector<Vector3>();
		}
	
	}
	cout << "end";
	path.push_back(start);
	path.invert();
    return path;
}

bool is_equal(GridMap::IndexKey ik1, GridMap::IndexKey ik2){
	return ik1.x == ik2.x && ik1.y == ik2.y && ik1.z == ik2.z;
}

void MyGrid::update_autotile(){

	Map<int, List<Pair<Transform, GridMap::IndexKey> > > multimesh_items;

	for (Map<GridMap::IndexKey, Cell>::Element *E = cell_map.front(); E; E = E->next()) {

		int id = E->get().item;
		if (id == 0){
			const GridMap::Cell &c = E->get();

			if (!basics_library.is_valid())
				continue;

			Vector3 cellpos = Vector3(E->key().x, E->key().y, E->key().z);
			Vector3 ofs = _get_offset();

			if (!multimesh_items.has(c.item)) {
				multimesh_items[c.item] = List<Pair<Transform, IndexKey> >();
			}

			Transform xform;

			xform.basis.set_orthogonal_index(c.rot);
			xform.set_origin(cellpos * cell_size + ofs);
			xform.basis.scale(Vector3(cell_scale, cell_scale, cell_scale));

			Pair<Transform, GridMap::IndexKey> p;
			p.first = xform; // transform
			p.second = E->key(); // indexKey
			multimesh_items[10].push_back(p);
			multimesh_items[5].push_back(p);
		}
	}

	for (Map<int, List<Pair<Transform, GridMap::IndexKey> > >::Element *E = multimesh_items.front(); E; E = E->next()) {
		
		RID mm = VS::get_singleton()->multimesh_create();
		VS::get_singleton()->multimesh_allocate(mm, E->get().size(), VS::MULTIMESH_TRANSFORM_3D, VS::MULTIMESH_COLOR_NONE);
		basics_library->get_item_mesh(E->key())->surface_set_material(0, basics_material);
		VS::get_singleton()->multimesh_set_mesh(mm, basics_library->get_item_mesh(E->key())->get_rid());

		int idx = 0;
		for (List<Pair<Transform, GridMap::IndexKey> >::Element *F = E->get().front(); F; F = F->next()) {
			VS::get_singleton()->multimesh_instance_set_transform(mm, idx, F->get().first);
			idx++;
		}

		RID instance = VS::get_singleton()->instance_create();
		VS::get_singleton()->instance_set_base(instance, mm);

		if (is_inside_tree()) {
			VS::get_singleton()->instance_set_scenario(instance, get_world()->get_scenario());
			VS::get_singleton()->instance_set_transform(instance, get_global_transform());
		}

	}
	




}