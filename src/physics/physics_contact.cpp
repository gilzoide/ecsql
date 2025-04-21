#include "../ecsql/component.hpp"

ecsql::Component ContactComponent {
	"Contact",
	{
		"shape1 REFERENCES entity(id) ON DELETE CASCADE",
		"shape2 REFERENCES entity(id) ON DELETE CASCADE",
		"normal_x",
		"normal_y",
	},
	"CREATE INDEX Contact_shape1_shape2 ON Contact(shape1, shape2)",
	true,
};
