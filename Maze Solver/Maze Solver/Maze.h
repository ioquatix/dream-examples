/*
 *  Maze.h
 *  Gang Land
 *
 *  Created by Samuel Williams on 14/09/06.
 *  Copyright 2006 Samuel G. D. Williams. All rights reserved.
 *
 */

#ifndef __MAZE_SOLVER_MAZE_H
#define __MAZE_SOLVER_MAZE_H

#include <Dream/Numerics/Vector.h>
#include <Dream/Numerics/Matrix.h>
#include <Dream/Simulation/PathFinder.h>

namespace MazeSolver {
	using namespace Dream;
	using namespace Dream::Numerics;
	using namespace Dream::Simulation;

	class Maze;

	typedef PathFinder<Maze, Vec2u, RealT> MazePathFinder;

	class Maze : public Object {
public:
		enum Wall {
			NORTH = 1,
			SOUTH = 2,
			EAST = 4,
			WEST = 8
		};

		static Vec2u wall_offset(Wall wall);
		static Wall opporsite_wall(Wall wall);

		struct Tile {
			Tile () {
				clear();
			}
			
			void clear () {
				walls = 0;
				floor = true;
				floor_color = vec(.25, .212, .73);
			}

			unsigned walls;
			bool floor;
			
			mutable Vec3 floor_color;
		};
		
		Vec2u _size;
		Tile * _tiles;
		
		Tile * at (const Vec2u & loc) {
			return _tiles + row_major_offset(loc[Y], loc[X], _size[WIDTH]);
		}
		
		const Tile * at (const Vec2u & loc) const {
			return _tiles + row_major_offset(loc[Y], loc[X], _size[WIDTH]);
		}

		void clear () {
			for (unsigned y = 0; y < _size[Y]; y++) {
				for (unsigned x = 0; x < _size[X]; x++) {
					Tile * tile = at(vec(x, y));
					
					tile->clear();
				}
			}
		}

		Maze (const Vec2u & size);
		const Vec2u & size() { return _size; }
		
		void read_maze (std::string data);
		
		void make_outside_walls ();
		void make_horizontal_wall (unsigned offsetFromTop, unsigned offsetFromLeft, int size, Wall wall);
		void make_vertical_wall (unsigned offsetFromLeft, unsigned offsetFromTop, int size, Wall wall);
		
		void generate_random_maze (unsigned seed);
		
		void notify_cost (const MazePathFinder::StepT& location, const MazePathFinder::CostT& cost_from_start, const MazePathFinder::CostT& cost_to_end);
		void add_steps_from (const MazePathFinder::Node *node, MazePathFinder & path_finder) const;
		MazePathFinder::CostT estimate_path_cost (const MazePathFinder::StepT& from_location, const MazePathFinder::StepT& to_location) const;
		MazePathFinder::CostT exact_path_cost (const MazePathFinder::StepT& from, const MazePathFinder::StepT& to) const;
		bool is_goal_state (MazePathFinder & path_finder, const MazePathFinder::StepT & from, const MazePathFinder::StepT &to) const;
	};

}

#endif
