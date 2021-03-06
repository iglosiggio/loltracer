materials {
	{
		shininess	= 0,
		diffuse		= (0, 0, 0),
		specular	= (0, 0, 0),
		ambient		= (0, 0, 0)
	},

	{
		shininess	= 4,
		diffuse		= (0.15, 0.22, 0.19),
		specular	= (0.02, 0.02, 0.02),
		ambient		= (0.15, 0.22, 0.19)
	},

	{
		shininess	= 25,
		diffuse		= (0.04, 0.03, 0.02),
		specular	= (0.05, 0.05, 0.05),
		ambient		= (0.04, 0.03, 0.02)
	}
}

scene {
	ambient {
		color = (0.1, 0.1, 0.1)
	},

	camera {
		point		= (0, 1, 3),
		direction	= (0, 0, -1),
		fov		= 90
	},

	smooth-union {
		smoothness	= 4,
		material	= #1,
		a =  sphere {
			point		= (0, 0, -4),
			radius		= 4
		},
		b =  sphere {
			point		= (0, 0, -12),
			radius		= 4
		}
	},

	plane {
		y		= -4,
		material	= #2
	},

	point_light {
		point			= (-6, 4, -4),
		diffuse_intensity	= (4, 3.5, 1.2),
		specular_intensity	= (4, 3.5, 1.2)
	},

	point_light {
		point			= (8, -1, -2),
		diffuse_intensity	= (1.2, 4, 3.5),
		specular_intensity	= (1.2, 4, 3.5)
	}
}
