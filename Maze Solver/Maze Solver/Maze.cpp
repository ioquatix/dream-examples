/*
 *  Maze.cpp
 *  Gang Land
 *
 *  Created by Samuel Williams on 14/09/06.
 *  Copyright 2006 Samuel G. D. Williams. All rights reserved.
 *
 */

#include "Maze.h"

#include <random>

namespace MazeSolver {

	Vec2u Maze::wall_offset(Wall wall) {
		switch (wall) {
			case NORTH:
				return Vec2u(0, 1);
			case SOUTH:
				return Vec2u(0, -1);

			case EAST:
				return Vec2u(1, 0);
			case WEST:
				return Vec2u(-1, 0);
		}
	}

	Maze::Wall Maze::opporsite_wall(Wall wall) {
		switch (wall) {
			case NORTH:
				return SOUTH;
			case SOUTH:
				return NORTH;

			case EAST:
				return WEST;
			case WEST:
				return EAST;
		}
	}

	Maze::Maze (const Vec2u & size) : _size(size) {
		_tiles = new Tile[_size.product()];
	}
	
	void Maze::read_maze (std::string data) {
		int row = 0, column = 0;
		bool eol = false;

		for (auto c : data) {
			if (c == '\n' || c == '\r') {
				eol = true;
				continue;
			} else if (eol) {
				eol = false;
				row += 1;
				column = 0;
			}
			
			if (c == '_') {
				make_horizontal_wall(row, (column-1) / 2, 1, SOUTH);
			} else if (c == '|') {
				make_vertical_wall(column / 2, (row-1), 1, WEST);
			}
			
			column += 1;
		}
	}
	
	void Maze::make_outside_walls () {
		make_horizontal_wall (0, 0, _size[WIDTH], SOUTH);
		make_horizontal_wall (_size[HEIGHT]-1, 0, _size[WIDTH], NORTH);
		make_vertical_wall (0, 0, _size[HEIGHT], WEST);
		make_vertical_wall (_size[WIDTH]-1, 0, _size[HEIGHT], EAST);
	}
	
	void Maze::make_horizontal_wall (unsigned offsetFromTop, unsigned offsetFromLeft, int size, Wall wall) {
		Vector<2, unsigned> l(_size[Y], 1);
		
		for (int i = 0; i < size; ++i) {
			Vec2u offset(offsetFromLeft+i, offsetFromTop);

			at(offset)->walls |= wall;
		}
	}
	
	void Maze::make_vertical_wall (unsigned offsetFromLeft, unsigned offsetFromTop, int size, Wall wall) {
		Vector<2, int> l(_size[Y], 1);
		
		for (int i = 0; i < size; ++i) {
			Vec2u offset(offsetFromLeft, offsetFromTop+i);
			
			at(offset)->walls |= wall;
		}
	}
	
	void Maze::add_steps_from (const MazePathFinder::Node *node, MazePathFinder & path_finder) const {		
		//std::cout << "Location: " << location << std::endl;
		//std::cout << "Left(" << location << "): " << at(location)->side << " Bottom (" << location << "): " << at(location)->bottom;
		//std::cout << " Right(" << location + vector(1, 0) << "): " << at(location + vector(1, 0))->bottom << " Top(" << location + vector(0, 1) << "): " << at(location + vector(0, 1))->side;
		//std::cout << std::endl;
		
		Vec2i location = node->step;

		for (unsigned i = 0; i < 4; i += 1) {
			Wall wall = Wall(1 << i);
			Vec2i step_location = location + wall_offset(wall);

			if (!step_location.greater_than_or_equal(Vec2i(ZERO)))
				continue;

			if (!step_location.less_than(_size))
				continue;

			if (at(location)->walls & wall)
				continue;

			if (at(step_location)->walls & opporsite_wall(wall))
				continue;

			path_finder.add_step(step_location, node);
		}
	}
	
	MazePathFinder::CostT Maze::estimate_path_cost (const MazePathFinder::StepT& from_location, const MazePathFinder::StepT& to_location) const {
		//std::cout << "|" << from_location << " => " << to_location << "| = " << (to_location - from_location).length() << std::endl;
		RealT cost = (to_location - from_location).length();
		if (cost > 1.0)
			return cost * 2.0;
		else
			return cost;
	}
	
	void Maze::notify_cost (const MazePathFinder::StepT& location, const MazePathFinder::CostT& cost_from_start, const MazePathFinder::CostT& cost_to_end) {
		RealT scale = (cost_from_start) / _size.sum();
		at(location)->floor_color = Vec3(0.1, 1.0, 0.5) * scale;
	}

	MazePathFinder::CostT Maze::exact_path_cost (const MazePathFinder::StepT& from, const MazePathFinder::StepT& to) const {
		return estimate_path_cost(from, to);
	}

	bool Maze::is_goal_state (MazePathFinder & path_finder, const MazePathFinder::StepT & from, const MazePathFinder::StepT & goal) const {
		return from == goal;
	}

	void Maze::generate_random_maze (unsigned seed) {
		make_outside_walls();

		//make_horizontal_wall(5, 0, 15, SOUTH);
		//return;

		std::mt19937 rng(seed);
		auto r6 = std::bind(std::uniform_int_distribution<int>(0, 6), rng);
		auto rj = std::bind(std::uniform_int_distribution<int>(0, _size.product()), rng);

		for (int i = 0; i < _size.product() * 1.5; ++i) {
			int p = r6();
			
			//std::cout << "Random: " << r.between(0.0, 2.0) << std::endl;
			if (p > 4)
				continue;
			else
				_tiles[rj()].walls |= 1 << p;
		}
	}
};
