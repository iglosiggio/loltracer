materials {
	{
		shininess	= 0,
		diffuse		= (0, 0, 0),
		specular	= (0, 0, 0),
		ambient		= (0, 0, 0)
	},

	{
		shininess	= 16,
		diffuse		= (0.15, 0.22, 0.19),
		specular	= (0.08, 0.08, 0.08),
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
		color = (0.03, 0.03, 0.03)
	},


	camera {
		point		= (-2, 6, 3),
		direction	= (0.3, -0.7, -1),
		fov		= 150
	},

	point_light {
		point			= (-2, 10, -1),
		diffuse_intensity	= (4, 4, 4),
		specular_intensity	= (4, 4, 4)
	},

	point_light {
		point			= (-7, 2, -5),
		diffuse_intensity	= (1, 1.5, 2),
		specular_intensity	= (1, 1.5, 2)
	},

	smooth-union {
		smoothness	= 3,
		material	= #1,
		a = smooth-union {
			smoothness	= 3,
			a =  sphere {
				point		= (0, 1, -6),
				radius		= 1
			},
			b = sphere {
				point		= (-1, 0.5, -3),
				radius		= 3
			}
		},
		b = smooth-union {
			smoothness	= 3,
			a = sphere {
				point		= (-3, 4.5, -3),
				radius		= 0.5
			},
			b = smooth-union {
				smoothness	= 3,
				a = sphere {
					point		= (2, 2, -10),
					radius		= 2
				},
				b = sphere {
					point		= (6, 2, -10),
					radius		= 5
				}
			}
		}
	},

	plane {
		y		= -1,
		material	= #2
	}
}
