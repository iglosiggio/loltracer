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
	},

	{
		shininess	= 2,
		diffuse		= (0, 0, 0.2),
		specular	= (0.01, 0.01, 0.01),
		ambient		= (0, 0, 0.2)
	},

	{
		shininess	= 10,
		diffuse		= (0.2, 0.2, 0),
		specular	= (0.001, 0.001, 0.001),
		ambient		= (0.2, 0.2, 0)
	}
}

scene {
	ambient {
		color = (0.03, 0.03, 0.03)
	},


	camera {
		point		= (0, 0, 0),
		direction	= (0, 0, -1),
		fov		= 150
	},

	point_light {
		point			= (-2, 10, -1),
		diffuse_intensity	= (4, 4, 4),
		specular_intensity	= (4, 4, 4)
	},

	sphere {
		point		= (0, 1, -6),
		radius		= 1,
		material	= #1
	},

	sphere {
		point		= (-1, 0.5, -3),
		radius		= 1,
		material	= #2
	},

	box {
		point		= (2, 2, -10),
		point2 		= (2, 2, 2),
		radius		= 0.6,
		material	= #3
	},

	plane {
		y		= -1,
		material	= #4
	}
}
