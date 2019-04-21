#pragma once
#include "Element/Element.h"
class Sand :
	public Element
{
public:
	Element* clone() const override;
	Sand(Simulation& sim);
	Sand(const Sand& rhs);
	~Sand();
};
