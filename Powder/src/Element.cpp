#include "Element.h"
#include "Simulation.h"
#include "Random.h"
#include <math.h>

Element* Element::move(Vector dest)
{
	pos = dest;
	Element* coll_el = this;
	int xD = static_cast<int>(std::round(dest.x));
	int yD = static_cast<int>(std::round(dest.y));
	int old_x = x;
	int old_y = y;
	if (x == xD && y == yD)
		return coll_el;
	int xStep, yStep;
	int dx = xD - x;
	int dy = yD - y;
	if (dy < 0)
	{
		yStep = -1;
		dy = -dy;
	}
	else
	{
		yStep = 1;
	}
	if (dx < 0)
	{
		xStep = -1;
		dx = -dx;
	}
	else
	{
		xStep = 1;
	}
	int ddx = 2 * dx; // double for more precision
	int ddy = 2 * dy;
	// we check if the slope is in the 1st 4th 5th and 8th octant
	if (ddx >= ddy)
	{
		coll_el = move_helper(x, y, dx, xStep, yStep, ddx, ddy, false);
	}
	else // if its in the other 4 octants
	{
		coll_el = move_helper(x, y, dy, xStep, yStep, ddy, ddx, true);
	}
	sim->gravity->update_mass(mass, x, y, old_x, old_y);
	return coll_el;
}

Element* Element::move_helper(int xO, int yO, int d, int xStep, int yStep, int de, int dr, bool ytype)
{
	Element* coll_el = this;
	int eprev = d, e = d;
	int diff_x, diff_y;
	for (int i = 0; i < d; i++)
	{
		ytype ? yO += yStep : xO += xStep;
		e += dr;
		if (e > de)
		{
			ytype ? xO += xStep : yO += yStep;
			e -= de;
			if (e + eprev < de)  // bottom square also
			{
				diff_x = xO - (ytype ? xStep : 0);
				diff_y = yO - (!ytype ? yStep : 0);
				coll_el = do_move(diff_x, diff_y);
				if (collision)
					break;
			}
			else if (e + eprev > de) // left corner
			{
				diff_x = xO - (!ytype ? xStep : 0);
				diff_y = yO - (ytype ? yStep : 0);
				coll_el = do_move(diff_x, diff_y);
				if (collision)
					break;
			}
			else //the corner case
			{
				// TODO
			}
		}
		coll_el = do_move(xO, yO);
		if (collision)
			break;
		eprev = e;
	}
	return coll_el;
}

Element* Element::do_move(int diff_x, int diff_y)
{
	Element* coll_el = this;
	if (!sim->bounds_check(diff_x, diff_y))
	{
		ground_coll.x = diff_x;
		ground_coll.y = diff_y;
		set_pos(x, y, true);
		collision = true;
	}
	else
	{
		coll_el = sim->get_from_grid(diff_x, diff_y);
		// if there is no collision we update the elements real coordinates
		if (coll_el->identifier != EL_NONE) //todo function that evals the collision
		{
			set_pos(x, y, true);
			collision = true;
		}
		else
		{
			sim->swap_elements(x, y, diff_x, diff_y);
			set_pos(diff_x, diff_y, false);
			coll_el = this;
		}
	}
	
	return coll_el;
}


void Element::calc_loads()
{
	forces.Zero();
	collision = false;
	forces += sim->gravity->get_force(x, y, mass);
	Vector air_drag;
	air_drag = -velocity;
	// our y in the grid increases downwards
	// as opposed to the upward increase in the normal cartesian grid
	// and the velocity y is the one we use not the normal cartesian
	air_drag.y = -air_drag.y;
	air_drag.Normalize();
	air_drag *= 0.5 * sim->air_density * speed * speed *
	(0.5) * drag_coef;
	forces += air_drag;
}

void Element::update_velocity(double dt)
{
	calc_loads();
	Vector a;
	a = forces / mass;
	// our y in the grid increases downwards
	// as opposed to the upward increase in the normal cartesian grid
	a.y = -a.y;
	add_velocity(a * dt);
	/*if (speed > terminal_vel)
	{
		velocity.Normalize();
		velocity *= terminal_vel;
	}*/
}

void Element::calc_term_vel() 
{
	terminal_vel_v = 2 * sim->gravity->base_grav * mass / sim->air_density * 1 * drag_coef;
	terminal_vel_v.x = sqrt(abs(terminal_vel_v.x));
	terminal_vel_v.y = sqrt(abs(terminal_vel_v.y));
	terminal_vel = terminal_vel_v.Magnitude();
}

void Element::set_pos(int x, int y, bool true_pos)
{
	this->x = x;
	this->y = y;
	if(true_pos)
	{ 
		pos.x = x;
		pos.y = y;
	}
}

bool Element::powder_pile()
{
	bool status = false;
	if (speed > pile_threshold)
	{
		Vector vel_norm = Vector::Normalize(velocity);
		Vector check_pos = vel_norm + pos;
		Vector perp = vel_norm.PerpendicularCW();
		Element* res = sim->get_from_grid(check_pos.x, check_pos.y);
		if (res && res->identifier != EL_NONE)
		{
			bool side = random.next_bool();
			status = pile_helper(check_pos + (side ? perp : -perp));
			if (!status)
				status = pile_helper(check_pos + (side ? -perp : perp));
		}
	}
	return status;
}

bool Element::pile_helper(Vector check_pos)
{
	Element* res = sim->get_from_grid(check_pos.x, check_pos.y);
	if (res && res->identifier == EL_NONE)
	{
		move(check_pos);
		return true;
	}
	return false;
}

void Element::apply_impulse(Element* coll_el, double dt)
{
	bool ground = (coll_el == this);
	Vector vr = (ground ? velocity : velocity - coll_el->velocity);
	Vector d = (ground ? pos - ground_coll : pos - coll_el->pos);
	d.Normalize();
	double j = (-(1 + restitution) * (vr * d)) 
		/ ((d * d) * (1 / mass + (ground ? 0 : 1 / coll_el->mass)));
	add_velocity(j * d / mass);
	if (!ground)
	{
		coll_el->velocity -= (j * d) / coll_el->mass;
	}
}

void Element::add_velocity(Vector nvelociry)
{
	velocity += nvelociry;
	speed = velocity.Magnitude();
}
