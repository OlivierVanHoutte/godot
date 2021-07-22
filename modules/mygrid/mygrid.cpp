/* mygrid.cpp */

#include "mygrid.h"

#define diagonal_step_cost 1.414
#define M_PIl 3.141592653589793238462643383279502884L /* pi */

void MyGrid::_bind_methods() {

	ClassDB::bind_method(D_METHOD("find_path", "start", "end"), &MyGrid::find_path);
	ClassDB::bind_method(D_METHOD("update_range", "start", "walk_range"), &MyGrid::update_range);
	ClassDB::bind_method(D_METHOD("update_autotile"), &MyGrid::update_autotile);

	ClassDB::bind_method(D_METHOD("set_basics_library", "mesh_library"), &MyGrid::set_basics_library);
	ClassDB::bind_method(D_METHOD("get_basics_library"), &MyGrid::get_basics_library);

	ClassDB::bind_method(D_METHOD("hide_invisible_meshes"), &MyGrid::hide_invisible_meshes);
	ClassDB::bind_method(D_METHOD("show_invisible_meshes"), &MyGrid::show_invisible_meshes);
	ClassDB::bind_method(D_METHOD("hide_basics"), &MyGrid::get_show_basics);
	ClassDB::bind_method(D_METHOD("show_basics"), &MyGrid::set_show_basics);


	ClassDB::bind_method(D_METHOD("set_basics_mat", "mesh_library"), &MyGrid::set_basics_mat);
	ClassDB::bind_method(D_METHOD("get_basics_mat"), &MyGrid::get_basics_mat);
	ClassDB::bind_method(D_METHOD("set_outline_mat", "mesh_library"), &MyGrid::set_outline_mat);
	ClassDB::bind_method(D_METHOD("get_outline_mat"), &MyGrid::get_outline_mat);

	ClassDB::bind_method(D_METHOD("get_ray_intersect", "start", "dir"), &MyGrid::get_ray_intersect);

	ClassDB::bind_method(D_METHOD("clear_outline", "clear_path_cache"), &MyGrid::clear_outline, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("create_outline"), &MyGrid::create_outline);
	ClassDB::bind_method(D_METHOD("set_outline_mesh_library", "mesh_library_outline"), &MyGrid::set_outline_mesh_library);
	ClassDB::bind_method(D_METHOD("get_outline_mesh_library"), &MyGrid::get_outline_mesh_library);
	
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh_library_outline", PROPERTY_HINT_RESOURCE_TYPE, "MeshLibrary"), "set_outline_mesh_library", "get_outline_mesh_library");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "outline_material", PROPERTY_HINT_RESOURCE_TYPE, "Material"), "set_outline_mat", "get_outline_mat");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "basics_meshlib", PROPERTY_HINT_RESOURCE_TYPE, "MeshLibrary"), "set_basics_library", "get_basics_library");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "basics_material", PROPERTY_HINT_RESOURCE_TYPE, "Material"), "set_basics_mat", "get_basics_mat");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "gen"), "show_basics", "hide_basics");
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
	range = walk_range;
	in_range.clear();
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
        _add_in_range(key, rkey, q, walk_range);
        IndexKey lkey = key;
        lkey.x--;
        _add_in_range(key, lkey, q, walk_range);
        IndexKey fkey = key;
        fkey.z++;
        _add_in_range(key, fkey, q, walk_range);
        IndexKey bkey = key;
        bkey.z--;
      	_add_in_range(key, bkey, q, walk_range);
		
	}
}

void MyGrid::_add_in_range(IndexKey source, IndexKey target, vector<IndexKey> &q, float walk_range){
	if (is_walkable(target)) {
      	  add_in_range(source, target, q, walk_range);
	}else{
		target.y--;
		if (is_walkable(target)) {
			add_in_range(source, target, q, walk_range);
		}
	}
}

void MyGrid::add_in_range(IndexKey source, IndexKey target, vector<IndexKey> &q, float walk_range){
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
			//set_cell_item(target.x, target.y, target.z, 1);
		}
	}
};

bool MyGrid::is_walkable(IndexKey key){
	int i = get_cell_item(key.x, key.y, key.z);
	if (mesh_library->has_item(i)) {
		if (mesh_library->get_item_cell_type(i) == MeshLibrary::GHOST){
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
		return ( mesh_library->get_item_cell_type(i) == MeshLibrary::SOLID);
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

	//cout << "start";
	while (!is_equal(e_ik, s_ik)){
		path.push_back(Vector3(e_ik.x, e_ik.y, e_ik.z));

		if (in_range.has(e_ik)){
			if (is_equal(e_ik, in_range[e_ik].source)){
				//cout << "aha";
				return Vector<Vector3>();
			}
			e_ik = in_range[e_ik].source;
		}else{
			//cout << "aha";
			return Vector<Vector3>();
		}
	
	}
	//cout << "end";
	path.push_back(start);
	path.invert();
    return path;
}

bool is_equal(GridMap::IndexKey ik1, GridMap::IndexKey ik2){
	return ik1.x == ik2.x && ik1.y == ik2.y && ik1.z == ik2.z;
}

void MyGrid::update_autotile(){

	for (int i = 0; i < multimesh_instances.size(); i++) {

		VS::get_singleton()->free(multimesh_instances[i].instance);
		VS::get_singleton()->free(multimesh_instances[i].multimesh);
	}
	multimesh_instances.clear();

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

			IndexKey left = E->key();
			left.x -= 1;
			IndexKey right = E->key();
			right.x += 1;
			IndexKey front = E->key();
			front.z -= 1;
			IndexKey back = E->key();
			back.z += 1;

			// Decide mesh indices
			int count = 0;
			bool l = false;
			if (cell_map.has(left)){
				l = (cell_map[left].item == 0);
				count += int(l);
			}
			bool r = false;
			if (cell_map.has(right)){
				r = (cell_map[right].item == 0);
				count += int(r);
			}
			bool f = false;
			if (cell_map.has(front)){
				f = (cell_map[front].item == 0);
				count += int(f);
			}
			bool b = false;
			if (cell_map.has(back)){
				b = (cell_map[back].item == 0);
				count += int(b);

			}	
			int rot_index = 0;
			int grass_index = -1;
			int wall_index = -1;
			switch (count)
			{
			case 0:
				grass_index = 5;
				wall_index = 10;
				break;
			case 1:
				rot_index = int(b)*16 + int(f)*22 + int(r)*10;
				grass_index = 3;
				wall_index = 8;
				break;
			case 2:
				if (l == r){
					grass_index = 4;
					wall_index = 9;
					if (l){
						rot_index = 0;
					}else{
						rot_index = 16;
					}

				}else{
					grass_index = 2;
					wall_index = 7;
					rot_index = int(l && b)*16 + int(r && f)*22 + int(r && b)*10;
				}
				break;
			case 3:
				rot_index = int(!r)*16 + int(!l)*22 + int(!f)*10;
				grass_index = 1;
				wall_index = 6;
				break;
			case 4:
				grass_index = 0;
				wall_index = -1;
				break;
			default:
				grass_index = 5;
				wall_index = 10;
				break;
			}

			IndexKey up = E->key();
			up.y += 1;
			if (cell_map.has(up)){
				if (cell_map[up].item == 0){
					grass_index = -1;

				}
			}
		
			Transform xform;

			xform.basis.set_orthogonal_index(rot_index);
			xform.set_origin(cellpos * cell_size + ofs);
			xform.basis.scale(Vector3(cell_scale, cell_scale, cell_scale));
			xform.translate(Vector3(0.0, -cell_size.y/2.0, 0.0));

			Pair<Transform, GridMap::IndexKey> p;
			p.first = xform; // transform
			p.second = E->key(); // indexKey
			if (grass_index >= 0){
				multimesh_items[grass_index].push_back(p);
			}
			if (wall_index >= 0){
				Transform xform2  = xform.translated(Vector3(0.0, -0.001, 0.0));
				Pair<Transform, GridMap::IndexKey> p1;
				p1.first = xform; // transform
				p1.second = E->key(); // indexKey
				multimesh_items[wall_index].push_back(p1);
				
				if (xform.origin.y <= 1){
					for (int i = 0; i < 4; i ++){
						xform2 = xform2.translated(Vector3(0.0, -cell_size.y, 0.0));
						Pair<Transform, GridMap::IndexKey> p2;
						p2.first = xform2; // transform
						p2.second = E->key(); // indexKey
						multimesh_items[wall_index].push_back(p2);
					}
				}

			}

			bool ld = false;
			IndexKey left_down = left;
			left_down.y -= 1;
			if (cell_map.has(left_down)){
				ld = (cell_map[left_down].item == 0);
			}
			bool fd = false;
			IndexKey front_down = front;
			front_down.y -= 1;
			if (cell_map.has(front_down)){
				fd = (cell_map[front_down].item == 0);
			}
			bool rd = false;
			IndexKey right_down = right;
			right_down.y -= 1;
			if (cell_map.has(right_down)){
				rd = (cell_map[right_down].item == 0);
			}
			bool bd = false;
			IndexKey back_down = back;
			back_down.y -= 1;
			if (cell_map.has(back_down)){
				bd = (cell_map[back_down].item == 0);
			}

			if (!l){
				if (!f){	
					if (fd || ld){
						Pair<Transform, GridMap::IndexKey> pg;
						Transform xform3 = xform;
						xform3.basis.set_orthogonal_index(10);
						pg.first = xform3.translated(Vector3(0.0, -cell_size.y, 0.0)); // transform
						pg.second = E->key(); // indexKey
						multimesh_items[11].push_back(pg);
					}
				}
				if (!b){
					if (bd || ld){
						Pair<Transform, GridMap::IndexKey> pg;
						Transform xform3 = xform;
						xform3.basis.set_orthogonal_index(22);
						pg.first = xform3.translated(Vector3(0.0, -cell_size.y, 0.0)); // transform
						pg.second = E->key(); // indexKey
						multimesh_items[11].push_back(pg);
					}
				}
			}
			if (!r){
				if (!f){
					if (fd || rd){
						Pair<Transform, GridMap::IndexKey> pg;
						Transform xform3 = xform;
						xform3.basis.set_orthogonal_index(16);
						pg.first = xform3.translated(Vector3(0.0, -cell_size.y, 0.0)); // transform
						pg.second = E->key(); // indexKey
						multimesh_items[11].push_back(pg);
					}
				}
				if (!b){
					if (bd || rd){
						Pair<Transform, GridMap::IndexKey> pg;
						Transform xform3 = xform;
						xform3.basis.set_orthogonal_index(0);
						pg.first = xform3.translated(Vector3(0.0, -cell_size.y, 0.0)); // transform
						pg.second = E->key(); // indexKey
						multimesh_items[11].push_back(pg);
					}
				}
			}
		
		}
	}

	for (Map<int, List<Pair<Transform, GridMap::IndexKey> > >::Element *E = multimesh_items.front(); E; E = E->next()) {
		Octant::MultimeshInstance mmi;
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
		
		//VS::get_singleton()->instance_geometry_set_cast_shadows_setting(basics_library->get_item_mesh(E->key())->get_rid(), VS::SHADOW_CASTING_SETTING_ON);
		if (is_inside_tree()) {
			VS::get_singleton()->instance_set_scenario(instance, get_world()->get_scenario());
			VS::get_singleton()->instance_set_transform(instance, get_global_transform());
		}
		VS::get_singleton()->instance_geometry_set_cast_shadows_setting(instance, VS::SHADOW_CASTING_SETTING_ON);
		mmi.instance = instance;
		mmi.multimesh = mm;
		multimesh_instances.push_back(mmi);

	}
	
}

void MyGrid::hide_invisible_meshes(){
	for (Map<OctantKey, Octant *>::Element *F = octant_map.front(); F; F = F->next()){
		if (!(F->get()->multimesh_instances.empty())){
			VS::get_singleton()->instance_set_visible ( F->get()->multimesh_instances[0].instance, false );
			//VS::get_singleton()->instance_geometry_set_cast_shadows_setting( F->get()->multimesh_instances[0].instance, VS::SHADOW_CASTING_SETTING_SHADOWS_ONLY );
		}
	}

}

void MyGrid::show_invisible_meshes(){
	for (Map<OctantKey, Octant *>::Element *F = octant_map.front(); F; F = F->next()){
		if (!(F->get()->multimesh_instances.empty())){
			VS::get_singleton()->instance_set_visible ( F->get()->multimesh_instances[0].instance, true );
		}
	}

}

void MyGrid::set_show_basics(bool t){
	if (t){
		show_basics();
	}else{
		hide_basics();
	}
	showing_basics = t;
}

bool MyGrid::get_show_basics(){
	return showing_basics;
}

void MyGrid::show_basics(){
	update_autotile();
	hide_invisible_meshes();
}

void MyGrid::hide_basics(){
	for (int i = 0; i < multimesh_instances.size(); i++) {

		VS::get_singleton()->free(multimesh_instances[i].instance);
		VS::get_singleton()->free(multimesh_instances[i].multimesh);
	}
	multimesh_instances.clear();
	show_invisible_meshes();
}

float MyGrid::get_ray_intersect(Vector3 start, Vector3 dir){
	dir = dir.normalized();
	start -= _get_offset() + Vector3(0.0, 0.8, 0.0);
	float tox = (dir.x > 0? 0: -1);
	float toy = (dir.y > 0? 0: -1);
	float toz = (dir.z > 0? 0: -1);
	Vector3 to = Vector3(tox, toy, toz);
	if (dir.x == 0){
		dir.x = 0;
	}
	if (dir.y == 0){
		dir.y = 0;
	}
	if (dir.z == 0){
		dir.z = 0;
	}

	Vector3 d = dir.sign();
	d.x = d.x > 0? 1: -1;
	d.y = d.y > 0? 1: -1;
	d.z = d.z > 0? 1: -1;

	Vector3 tc = (start/cell_size).floor() + Vector3(1.0, 1.0, 1.0);
	IndexKey last_ik;
	last_ik.x = tc.x;
	last_ik.y = tc.y;
	last_ik.z = tc.z;
	float t = 0;
	Vector3 cur = start;

	if (dir.length() == 0){
		return INFINITY;
	}

	while (t < 40.0){
		
		IndexKey ik;
		ik.x = tc.x;
		ik.y = tc.y;
		ik.z = tc.z;
		if (cell_map.has(ik)){
			if (cell_map[ik].item == 0){
				return t;
			}
		}
		last_ik = ik;
		Vector3 dtv = ((tc + to)*cell_size - cur) / dir;
		
		float dt = dtv.x;
		if (dtv.y < dt){
			dt = dtv.y;
		}
		if (dtv.z < dt){
			dt = dtv.z;
			tc.z += d.z;
		}
		if (dt == dtv.x){
			tc.x += d.x;
		}
		if (dt == dtv.y){
			tc.y += d.y;
		}
		t += dt;
		cur = start + t*dir;

	}

	return INFINITY;

}

void MyGrid::clear_outline(bool clear_path_cache) {
	for (int i = 0; i < outline_multimeshes.size(); i++) {

		VS::get_singleton()->free(outline_multimeshes[i].instance);
		VS::get_singleton()->free(outline_multimeshes[i].multimesh);
	}

	if (!outline_multimeshes.empty()) {
		outline_multimeshes.clear();
	}

	if (clear_path_cache) {
		in_range.clear();
		//paths.clear();
	}
	outline_visible = false;
}

void MyGrid::create_outline() {

	clear_outline(false);

	// Draw Outline
	if (!mesh_library_outline.is_null()) {
		int total_outline_meshes = 0;

		Map<int, List<Pair<Transform, IndexKey> > > outline_sorted;
		for (Map<IndexKey, DistanceTo>::Element *E = in_range.front(); E; E = E->next()) {
			int touching = 0;
			IndexKey ik = E->key();
			IndexKey ik_right = IndexKey(ik.x + 1, ik.y, ik.z);
			IndexKey ik_left = IndexKey(ik.x - 1, ik.y, ik.z);
			IndexKey ik_up = IndexKey(ik.x, ik.y, ik.z - 1);
			IndexKey ik_down = IndexKey(ik.x, ik.y, ik.z + 1);
			bool right = in_range.has(ik_right);
			bool left = in_range.has(ik_left);
			bool up = in_range.has(ik_up);
			bool down = in_range.has(ik_down);
			
			/*bool right = false;
			bool left = false;
			bool up = false;
			bool down = false;*/
			Vector3 cellpos = Vector3(ik.x, ik.y, ik.z);
			Transform xform;
			xform.basis.set_orthogonal_index(0);
			Vector3 ofs = (_get_offset() - get_global_transform().origin)/cell_size;
			ofs.y -= (- 0.5);
			ofs.z -= 0.5;
			ofs.x -= 0.5;

			Vector3 scale = Vector3(1.0, 1.0, 1.333);

			if (!up) {
				Transform xx = xform;
				Vector3 ff = ofs;
				int idx = 1;
				if (left) {
					IndexKey ik_ul = IndexKey(ik.x - 1, ik.y, ik.z - 1);
					if (in_range.has(ik_ul)) {
						ff.x += 0.1;
						idx ++;
					}
				} else {
					ff.x += 0.1;
					idx ++;
					add_outline_part(outline_sorted, 0, M_PIl, ik, ofs, cellpos, xform); // C_O
				}
				if (right) {
					IndexKey ik_ur = IndexKey(ik.x + 1, ik.y, ik.z - 1);
					if (in_range.has(ik_ur)) {
						ff.x -= 0.1;
						idx ++;
					}
				} else {
					ff.x -= 0.1;
					idx ++;
					add_outline_part(outline_sorted, 0, M_PIl / 2.0, ik, ofs, cellpos, xform); // C_O
				}
				add_outline_part(outline_sorted, idx, M_PIl / 2.0, ik, ff, cellpos, xx); // L

			} else {
				if (left) {
					IndexKey ik_ul = IndexKey(ik.x - 1, ik.y, ik.z - 1);
					if (!in_range.has(ik_ul)) {
						add_outline_part(outline_sorted, 4, M_PIl / 2.0, ik, ofs, cellpos, xform); // C_I
					}
				}
				if (right) {
					IndexKey ik_ur = IndexKey(ik.x + 1, ik.y, ik.z - 1);
					if (!in_range.has(ik_ur)) {
						add_outline_part(outline_sorted, 4, 0.0, ik, ofs, cellpos, xform); // C_I
					}
				}
			}
			if (!down) {
				Transform xx = xform;
				Vector3 ff;
				ff.x = ofs.x;
				ff.y = ofs.y;
				ff.z = ofs.z;
				int idx = 1;
				if (left) {
					IndexKey ik_dl = IndexKey(ik.x - 1, ik.y, ik.z + 1);
					if (in_range.has(ik_dl)) {
						ff.x += 0.1;
						idx ++;
					}
				} else {
					ff.x += 0.1;
					idx ++;
					add_outline_part(outline_sorted, 0, -M_PIl / 2.0, ik, ofs, cellpos, xform); // C_O
				}
				if (right) {
					IndexKey ik_dr = IndexKey(ik.x + 1, ik.y, ik.z + 1);
					if (in_range.has(ik_dr)) {
						ff.x -= 0.1;
						idx ++;
					}
				} else {
					ff.x -= 0.1;
					idx ++;
					add_outline_part(outline_sorted, 0, 0.0, ik, ofs, cellpos, xform); // C_O
				}
				add_outline_part(outline_sorted, idx, -M_PIl / 2.0, ik, ff, cellpos, xx); // L
			} else {
				if (left) {
					IndexKey ik_dl = IndexKey(ik.x - 1, ik.y, ik.z + 1);
					if (!in_range.has(ik_dl)) {
						add_outline_part(outline_sorted, 4, M_PIl, ik, ofs, cellpos, xform); // C_I
					}
				}
				if (right) {
					IndexKey ik_dr = IndexKey(ik.x + 1, ik.y, ik.z + 1);
					if (!in_range.has(ik_dr)) {
						add_outline_part(outline_sorted, 4, -M_PIl / 2.0, ik, ofs, cellpos, xform); // C_I
					}
				}
			}

			if (!right) {
				Transform xx = xform;
				Vector3 ff;
				ff.x = ofs.x;
				ff.y = ofs.y;
				ff.z = ofs.z;
				int idx = 1;
				if (up) {
					IndexKey ik_ur = IndexKey(ik.x + 1, ik.y, ik.z - 1);
					if (in_range.has(ik_ur)) {
						ff.z += 0.1;
						idx ++;
					}
				}else{
					ff.z += 0.1;
					idx ++;
				}
				if (down) {
					IndexKey ik_dr = IndexKey(ik.x + 1, ik.y, ik.z + 1);
					if (in_range.has(ik_dr)) {
						ff.z -= 0.1;
						idx ++;
					}
				}else{
					ff.z -= 0.1;
					idx ++;
				}
				add_outline_part(outline_sorted, idx, 0.0, ik, ff, cellpos, xx); // L
			}
			if (!left) {
				Transform xx = xform;
				Vector3 ff;
				ff.x = ofs.x;
				ff.y = ofs.y;
				ff.z = ofs.z;
				int idx = 1;
				if (up) {
					IndexKey ik_ul = IndexKey(ik.x - 1, ik.y, ik.z - 1);
					if (in_range.has(ik_ul)) {
						ff.z += 0.1;
						idx ++;
					}
				}else{
					ff.z += 0.1;
					idx ++;
				}
				if (down) {
					IndexKey ik_dl = IndexKey(ik.x - 1, ik.y, ik.z + 1);
					if (in_range.has(ik_dl)) {
						ff.z -= 0.1;
						idx ++;
					}
				}else{
					ff.z -= 0.1;
					idx ++;
				}
				add_outline_part(outline_sorted, idx, M_PIl, ik, ff, cellpos, xx); // L
			}
		}

		for (Map<int, List<Pair<Transform, IndexKey> > >::Element *E = outline_sorted.front(); E; E = E->next()) {
			RID mm = VS::get_singleton()->multimesh_create();
			VS::get_singleton()->multimesh_allocate(mm, outline_sorted[E->key()].size(), VS::MULTIMESH_TRANSFORM_3D, VS::MULTIMESH_COLOR_NONE);
			VS::get_singleton()->multimesh_set_mesh(mm, mesh_library_outline->get_item_mesh(E->key())->get_rid());
			int idx = 0;
			

			for (List<Pair<Transform, IndexKey> >::Element *F = outline_sorted[E->key()].front(); F; F = F->next()) {

				VS::get_singleton()->multimesh_instance_set_transform(mm, idx, F->get().first);
#ifdef TOOLS_ENABLED

				Octant::MultimeshInstance::Item it;
				it.index = idx;
				it.transform = F->get().first;
				it.key = F->get().second;
#endif

				idx++;
			}

			RID instance = VS::get_singleton()->instance_create();
			VS::get_singleton()->instance_set_base(instance, mm);
			mesh_library_outline->get_item_mesh(E->key())->surface_set_material(0, outline_material);
			if (mesh_library_outline->get_item_material(E->key()).is_valid()){
				mesh_library_outline->get_item_mesh(E->key())->surface_set_material(0, mesh_library_outline->get_item_material(E->key()));
			}
			VS::get_singleton()->instance_geometry_set_cast_shadows_setting(instance, VS::SHADOW_CASTING_SETTING_OFF);
			if (is_inside_tree()) {
				VS::get_singleton()->instance_set_scenario(instance, get_world()->get_scenario());
				VS::get_singleton()->instance_set_transform(instance, get_global_transform());
			}
			Outline_MMI o_mmi;
			o_mmi.instance = instance;
			o_mmi.multimesh = mm;
			outline_multimeshes.push_back(o_mmi);
		}
	}

	outline_visible = true;
}

bool MyGrid::is_outline_visible() {
	return outline_visible;
}

void MyGrid::add_outline_part(Map<int, List<Pair<Transform, IndexKey> > > &outline_sorted, int index, float rotation, IndexKey ik, Vector3 ofs, Vector3 cellpos, Transform xform) {
	xform = xform.rotated(Vector3(0, 1, 0), rotation);
	xform = xform.scaled(cell_size);
	xform.set_origin(cellpos * cell_size + ofs * cell_size + Vector3(0.0, -0.8, 0.0));
	xform.basis.scale(Vector3(cell_scale, cell_scale, cell_scale));
	outline_sorted[index].push_back(Pair<Transform, IndexKey>(xform, ik));
}

void MyGrid::set_outline_mesh_library(const Ref<MeshLibrary> &p_mesh_library) {

	if (!mesh_library_outline.is_null())
		mesh_library_outline->unregister_owner(this);
	mesh_library_outline = p_mesh_library;
	if (!mesh_library_outline.is_null())
		mesh_library_outline->register_owner(this);

	_recreate_octant_data();
	_change_notify("mesh_library_outline");
}

Ref<MeshLibrary> MyGrid::get_outline_mesh_library() const {

	return mesh_library_outline;
}

void MyGrid::set_outline_mat(const Ref<Material> &p_mat){
	if (!outline_material.is_null())
		outline_material->unregister_owner(this);
	outline_material = p_mat;
	if (!outline_material.is_null())
		outline_material->register_owner(this);

	outline_material = p_mat;
}

Ref<Material> MyGrid::get_outline_mat() const{
	return outline_material;

}

MyGrid::~MyGrid(){

	clear_outline(true);
	for (int i = 0; i < multimesh_instances.size(); i++) {

		VS::get_singleton()->free(multimesh_instances[i].instance);
		VS::get_singleton()->free(multimesh_instances[i].multimesh);
	}
	multimesh_instances.clear();

	if (!mesh_library_outline.is_null())
		mesh_library_outline->unregister_owner(this);
	if (!outline_material.is_null())
		outline_material->unregister_owner(this);

	if (!basics_library.is_null())
		basics_library->unregister_owner(this);
	if (!basics_material.is_null())
		basics_material->unregister_owner(this);
		
	


}