#include "None_Element.h"


bool None_Element::update(double dt)
{
	return 0;
}

void None_Element::render(float cell_height, float cell_width, sf::Vertex * quad)
{
}

Element * None_Element::clone() const
{
	return new None_Element();
}

None_Element::None_Element()
{
	identifier = 0;
}


None_Element::~None_Element()
{
}
