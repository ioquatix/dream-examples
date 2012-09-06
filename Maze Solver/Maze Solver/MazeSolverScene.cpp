/*
 *  MazeSolverScene.cpp
 *  Dream
 *
 *  Created by Samuel Williams on 21/09/06.
 *  Copyright 2006 Samuel G. D. Williams. All rights reserved.
 *
 */

#include "MazeSolverScene.h"

namespace MazeSolver {
	void MazeSolverScene::will_become_current(ISceneManager * manager) {
		Scene::will_become_current(manager);
		//_grid = new Grid ();
		//_axes = new Axes ();

		_camera = new BirdsEyeCamera;
		Ref<IProjection> projection = new PerspectiveProjection(R90, 1.0, 1024.0);
		
		_renderer_state = new RendererState;
		_renderer_state->shader_manager = new ShaderManager;
		_renderer_state->texture_manager = new TextureManager;
		_renderer_state->resource_loader = manager->resource_loader();
		_renderer_state->viewport = new Viewport(_camera, projection);

		{
			_flat_program = _renderer_state->load_program("Shaders/flat");
			_flat_program->set_attribute_location("position", POSITION);
			_flat_program->set_attribute_location("normal", NORMAL);
			_flat_program->set_attribute_location("mapping", MAPPING);
			_flat_program->link();

			auto binding = _flat_program->binding();
			binding.set_texture_unit("diffuse_texture", 0);
		}

		{
			_wireframe_program = _renderer_state->load_program("Shaders/wireframe");
			_wireframe_program->set_attribute_location("position", WireframeRenderer::POSITION);
			_wireframe_program->link();

			_wireframe_renderer = new WireframeRenderer;
		}

		{
			using namespace Geometry;

			Shared<MeshT> mesh = new MeshT;
			mesh->layout = LINES;
			Generate::grid(*mesh, 32, 2.0);

			_grid_mesh_buffer = new MeshBuffer<MeshT>();
			_grid_mesh_buffer->set_mesh(mesh);

			{
				auto binding = _grid_mesh_buffer->vertex_array().binding();

				// Attach indices
				binding.attach(_grid_mesh_buffer->index_buffer());

				// Attach attributes
				auto attributes = binding.attach(_grid_mesh_buffer->vertex_buffer());
				attributes[POSITION] = &MeshT::VertexT::position;
				attributes[NORMAL] = &MeshT::VertexT::normal;
				attributes[MAPPING] = &MeshT::VertexT::mapping;
			}
		}

		{
			Shared<MeshT> mesh = new MeshT;
			Generate::plane(*mesh, Vec2(9.6, 9.6));
			mesh->layout = Geometry::TRIANGLE_FAN;

			_floor_mesh_buffer = new MeshBufferT(mesh);

			{
				auto binding = _floor_mesh_buffer->vertex_array().binding();

				// Attach indices
				binding.attach(_floor_mesh_buffer->index_buffer());

				// Attach attributes
				auto attributes = binding.attach(_floor_mesh_buffer->vertex_buffer());
				attributes[POSITION] = &MeshT::VertexT::position;
				attributes[NORMAL] = &MeshT::VertexT::normal;
				attributes[MAPPING] = &MeshT::VertexT::mapping;
			}
		}

		{
			Shared<MeshT> mesh = new MeshT;
			Generate::plane(*mesh, Vec2(9.6, 9.6));
			mesh->layout = Geometry::TRIANGLE_FAN;

			Mat44 transform = IDENTITY;
			transform = transform.translated_matrix(Vec3(-10, 0, 10));
			transform = transform * transform.rotating_matrix_around_y(R90);
			mesh->apply(transform);

			_wall_mesh_buffer = new MeshBufferT(mesh);

			{
				auto binding = _wall_mesh_buffer->vertex_array().binding();

				// Attach indices
				binding.attach(_wall_mesh_buffer->index_buffer());

				// Attach attributes
				auto attributes = binding.attach(_wall_mesh_buffer->vertex_buffer());
				attributes[POSITION] = &MeshT::VertexT::position;
				attributes[NORMAL] = &MeshT::VertexT::normal;
				attributes[MAPPING] = &MeshT::VertexT::mapping;
			}
		}

		_maze = new Maze(vec(40, 20));
		_seed = 48294;

		reset_maze();
		_last_find = 0;

		glEnable(GL_DEPTH_TEST);
	}

	void MazeSolverScene::will_revoke_current(ISceneManager * manager) {
		glDisable(GL_DEPTH_TEST);
		
		_renderer_state = nullptr;
		_maze = nullptr;

		Scene::will_revoke_current(manager);
	}

	bool MazeSolverScene::resize (const ResizeInput & input)
	{
		logger()->log(LOG_INFO, LogBuffer() << "Resizing to " << input.new_size());

		_renderer_state->viewport->set_bounds(AlignedBox<2>(ZERO, input.new_size()));
		glViewport(0, 0, input.new_size()[WIDTH], input.new_size()[HEIGHT]);

		check_graphics_error();

		return Scene::resize(input);
	}

	bool MazeSolverScene::motion (const MotionInput & input)
	{
		_camera->process(input);
		return true;
	}

	void MazeSolverScene::reset_maze () {
		if (_path_finder)
			_path_finder = nullptr;
		
		_maze->clear();
		
		const char* TestMazeData1 = ""
		" _ _ _ _ _ _ _ _ _ _\n"
		"|  _ _ _ _ _ _ _ _  |\n"
		"| | |  _| | | |_ _| |\n"
		"| | |  _  |      _| |\n"
		"| |_|_ _ _|_ _|_|_| |\n"
		"|_ _ _ _  |_ _ _ _| |\n"
		"| | |  _|_ _ _ _ _ _|\n"
		"|_| |_|  _ _ _|  _| |\n"
		"|_   _  |_     _|_  |\n"
		"| |_  |_  | |_ _| | |\n"
		"|_ _ _ _ _ _ _ _ _|_|\n";

		//_maze->read_maze(TestMazeData1);

		// Generate a random maze:
		_maze->generate_random_maze(++_seed);
		_finished = false;
		_last_find = manager()->event_loop()->stopwatch().time();

		// Set the goal:
		_path_finder = new MazePathFinder(vec(0, 0), vec(39, 19), _maze.get());
		_maze->at(vec(39, 19))->floor_color = vec<RealT>(1.0, 0.0, 0.0);
	}

	void MazeSolverScene::render_frame_for_time (TimeT time) {
		manager()->process_pending_events(this);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//lighting(true);
		//showFramesPerSecond();
		//_grid->render(renderer());
		//_maze->render(renderer());

		if (0) {
			auto binding = _wireframe_program->binding();
			binding.set_uniform("major_color", Vec4(1.0, 1.0, 1.0, 1.0));
			binding.set_uniform("model_matrix", Mat44(IDENTITY));

			// Draw the various objects to the screen:
			binding.set_uniform("display_matrix", _renderer_state->viewport->display_matrix());

			_grid_mesh_buffer->draw();
		}

		{
			// This is a bit of a hack but it seems to work "okay".
			auto binding = _wireframe_program->binding();
			binding.set_uniform("display_matrix", _renderer_state->viewport->display_matrix());

			Vec2u size = _maze->size();
			for (std::size_t y = 0; y < size[Y]; y += 1) {
				for (std::size_t x = 0; x < size[X]; x += 1) {
					Maze::Tile * tile = _maze->at(Vec2u(x, y));
					Vec2 center = (Vec2(x, y) - (size / 2)) * 20;

					{
						if (tile->floor)
							binding.set_uniform("major_color", tile->floor_color << 1.0);
						else
							binding.set_uniform("major_color", Vec4(1.0, 0.0, 0.0, 1.0));
						
						binding.set_uniform("model_matrix", Mat44::translating_matrix(center));
						_floor_mesh_buffer->draw();
					}

					binding.set_uniform("major_color", Vec4(0.5, 0.3, 0.8, 1.0));

					if (tile->walls & Maze::NORTH) {
						binding.set_uniform("model_matrix", Mat44::translating_matrix(center) * Mat44::rotating_matrix_around_z(-R90));
						_wall_mesh_buffer->draw();
					}

					if (tile->walls & Maze::SOUTH) {
						binding.set_uniform("model_matrix", Mat44::translating_matrix(center) * Mat44::rotating_matrix_around_z(R90));
						_wall_mesh_buffer->draw();
					}

					if (tile->walls & Maze::EAST) {
						binding.set_uniform("model_matrix", Mat44::translating_matrix(center) * Mat44::rotating_matrix_around_z(R180));
						_wall_mesh_buffer->draw();
					}

					if (tile->walls & Maze::WEST) {
						binding.set_uniform("model_matrix", Mat44::translating_matrix(center) * Mat44::rotating_matrix_around_z(0));
						_wall_mesh_buffer->draw();
					}
				}
			}
		}
		//_lightMesh->render(renderer());
		
		//if (_particleSystem->size() < 600) {
		//	ParticleSystem::particles_t particles = _particleSystem->add(5);
			
		//	for (iterateEach (particles, p)) {
		//		(*p)->setup(this, (_pathFinder->top()->step * 10 + 5) << 5);
		//	}
		//}
		
		//_particleSystem->update(this);
		//_particleSystem->render(renderer());
		
		if (time - _last_find > 0.05) {
			if (_path_finder->find_path(1)) {
				if (_finished) {
					logger()->log(LOG_INFO, "Maze finished, resetting...");
					reset_maze();
				} else {
					_finished = true;
				}
			}
			_last_find = time;
		}
		
		MazePathFinder::PathT steps;
		steps = _path_finder->construct_current_path();
		
		Path visual_path;
		Vec3 path_color(1.0, 0.2, 0.2);

		for (auto step : steps) {
			Vec3 point = (((step * 10) << 0) + 5);

			// Render points...
		}
	}

}
