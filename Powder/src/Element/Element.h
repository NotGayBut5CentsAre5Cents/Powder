#pragma once
#include <string>
#include <SFML/Graphics.hpp>
#include "ElementsIds.h"
#include "UI/ElementEditor.h"
#include "Utils/Vector.h"

enum ElementProperties : uint16_t
{
	NoProperties = 0,
	// when set to true, will use high_temperature
	// to transition to liquid state
	Meltable = 1 << 0,
	// if no high_temperature_transition is set
	// will use high_temperature for spontaneous combustion
	Flammable = 1 << 1,
	Tmp_velocity = 1 << 2,
	Explosive = 1 << 3,
	Corrosive = 1 << 4,
	Life_dependant = 1 << 5,
	Life_decay = 1 << 6,
	Burning = 1 << 7
};

constexpr ElementProperties operator& (ElementProperties x, ElementProperties y)
{
	return static_cast<ElementProperties>(
		static_cast<uint16_t>(x) & static_cast<uint16_t>(y));
}

constexpr ElementProperties operator| (ElementProperties x, ElementProperties y)
{
	return static_cast<ElementProperties>(
		static_cast<uint16_t>(x) | static_cast<uint16_t>(y));
}

constexpr ElementProperties operator^ (ElementProperties x, ElementProperties y)
{
	return static_cast<ElementProperties>(
		static_cast<uint16_t>(x) ^ static_cast<uint16_t>(y));
}

constexpr ElementProperties operator~ (ElementProperties x)
{
	return static_cast<ElementProperties>(~static_cast<uint16_t>(x));
}

constexpr ElementProperties& operator&= (ElementProperties& x, ElementProperties y)
{
	x = x & y; return x;
}

constexpr ElementProperties& operator|= (ElementProperties& x, ElementProperties y)
{
	x = x | y; return x;
}

constexpr ElementProperties& operator^= (ElementProperties& x, ElementProperties y)
{
	x = x ^ y; return x;
}


class Simulation;

class Element
{
public:
	int identifier = -1;
	std::string name = "TMP_ELEMENT";	// Will be used in the ui
	std::string description = "DESC";
	sf::Color color = sf::Color::Blue;	// The main color
	Simulation* sim; // pointer to the Simulation the element currently is 
	ElementEditor* editor = nullptr;
	int menu_id = -1;		// Will be used in the ui 
	int menu_section = -1;	//
	int x = 0; // Current position in the grid of elements
	int y = 0; //
	Vector pos = Vector(0, 0);
	Vector velocity;
	float speed = 0.0f;
	bool moved = false;
	Vector forces;
	// how much gravity affects gases
	// set to negative to make the effect of rising up
	float gas_gravity = 1.f;
	float gas_pressure = 0;
	float mass = 1;
	int endurance = 0;
	float life = 100.f;
	int damage = 0;
	float restitution = 0.6f;
	float temperature = 0; //in kelvins
	float thermal_cond = 0;
	float specific_heat_cap = 0;
	float flammability = 1.f;
	int state = 0; // 0 - gas 1 - liquid 2 - powder 3 - solid  
	ElementProperties prop = NoProperties;
	// Moves the element across a line (the start of the line is the 
	// x and y of the element itself and the end is xDestination and yDestination)
	// this function uses a modified version of bresenhams line algorithm
	// return true if there is no collision 
	Element* move(Vector dest);
	void element_copy(const Element& rhs);
	void update_velocity(float dt);
	void set_pos(int x, int y, bool true_pos);
	void apply_collision_impulse(Element* collided_elem, float dt);
	void add_velocity(Vector nvelocity);
	void add_heat(float heat);
	virtual int update(float dt);
	virtual void render(float cell_height, float cell_width, sf::Vertex* quad);
	virtual void draw_ui();
	virtual Element* clone() const = 0;
	virtual ~Element();
protected:
	int low_pressure_transition = -1;		// To which element the current element
	int high_pressure_transition = -1;		// will transfrom, based on the current
	int low_temperature_transition = -1;	// physical state of the element
	int high_temperature_transition = -1;	//
	float low_pressure = -300;                    // Number values at which the 
	float high_pressure = 300;					// transformation will occur
	float low_temperature = -1;
	float high_temperature = 10'000;
	float spotaneous_combustion_tmp = 10'000;
	// used by powders in the creation of piles
	// higher values means its harder for pile creation to occur
	// essentially used as velocity threshold
	// at which pile creation will happen
	int pile_threshold = 1;
	// damage threshold
	Vector ground_coll;
	Element* collided_elem;
	std::vector<sf::Color> colors;	// All the possible colors
	// 0 - block; the element is blocked from moving further
	// 1 - pass; both elements occupy the same space
	// 2 - swap; the elements switch places
	virtual int eval_col(Element* coll);
	void calc_loads();
	bool collision = false;
	void burn();
	void liquid_move();
	bool powder_pile();
private:
	void move_helper(int xO, int yO, int d, int xStep, int yStep, int de, int dr, bool ytype, Element*& coll_el);
	void do_move(int diff_x, int diff_y, Element*& coll_el);
};