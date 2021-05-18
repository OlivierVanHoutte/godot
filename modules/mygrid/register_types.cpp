/* register_types.cpp */

#include "register_types.h"

#include "core/class_db.h"
#include "mygrid.h"

void register_mygrid_types() {
	ClassDB::register_class<MyGrid>();
}

void unregister_mygrid_types() {
	// Nothing to do here in this example.
}