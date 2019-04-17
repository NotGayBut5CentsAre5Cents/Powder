#include "Water.h"
#include "Simulation.h"
#include "Utils/Random.h"

Element* Water::clone() const
{
	return new Water(*this);
}

Water::Water(Simulation& sim)
{
	identifier = EL_WATER;
	name = "Water";
	description = "Water";
	colors = {sf::Color::Blue};
	color = colors[0];
	mass = 1;
	restitution = 0.0f;
	temperature = 293.15f;
	thermal_cond = 0.606f;
	specific_heat_cap = 4.19f;
	state = ST_LIQUID;
	low_temperature = 273.15f;
	low_temperature_transition = EL_ICE;
	this->sim = &sim;
}

Water::Water(const Water& rhs)
{
	Element::element_copy(rhs);
	color = colors[random.between(0, colors.size() - 1)];
}


Water::~Water()
{
}
