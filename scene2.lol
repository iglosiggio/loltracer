materials {
	{
		shininess	= 4,
		diffuse		= (0, 0, 0),
		specular	= (0, 0, 0),
		ambient		= (0, 0, 0)
	},

	{
		shininess	= 3,
		diffuse		= (0.2, 0, 0),
		specular	= (0.2, 0.2, 0.2),
		ambient		= (0.2, 0, 0)
	},

	{
		shininess	= 50,
		diffuse		= (0, 0.2, 0),
		specular	= (0.2, 0.2, 0.2),
		ambient		= (0, 0.2, 0)
	}
}

scene {
	ambient {
		color = (0.01, 0.01, 0.01)
	},


	camera {
		point		= (0, 0, 0),
		nw_corner	= (-1, 1, 1),
		se_corner	= (1, -1, 1)
	},

	point_light {
		point			= (-2, 10, 1),
		diffuse_intensity	= (2, 2, 2),
		specular_intensity	= (2, 2, 2)
	},

	point_light {
		point			= (-3, 0, 5),
		diffuse_intensity	= (2, 2, 2),
		specular_intensity	= (2, 2, 2)
	},

	sphere {
		point		= (0, 1, 6),
		radius		= 1,
		material	= #1
	},

	sphere {
		point		= (-1, 0.5, 3),
		radius		= 1,
		material	= #1
	},

	sphere {
		point		= (5, -3, 10),
		radius		= 1,
		material	= #1
	},

	plane {
		y		= -6,
		material	= #2
	}
}
