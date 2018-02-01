#include <cmath>
#include <random>
#include <chrono>
#include <unordered_map>

#include <omp.h>
#include <Eigen/Dense>
#include <Eigen/Geometry>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "OpenGLHelper.h"
#include "CameraHelper.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define WINDOW_HEIGHT 960
#define WINDOW_WIDTH 1280 

struct Mesh {
	Eigen::Matrix<float, -1, -1> V;
	Eigen::Matrix<uint32_t, -1, -1> F;
};


struct Voxel {
	int32_t i, j, k;
};

struct RenderVAO {
	GLuint program;
	GLuint id_vao, id_vbo_vertex, id_vbo_normal, id_vbo_position;

};

struct VoxelizationVAO {
	GLuint program;
	GLuint id_vao, id_vbo_position, id_ebo, id_image_occupany, id_image_color;

};

class ToyWorld {
public:

    void term() {
        
    }
	
	void init() {
		load_mesh();
		init_voxelization_vao();
		init_render_vao();
	}
	
	void load_mesh() {
		tinyobj::attrib_t attrib;
        std::string filename = "/home/amon/grive/development/Voxelization/resources/bunny.obj";
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string err;
		bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename.c_str());
    	assert(ret);
		
		mesh.V.resize(3, attrib.vertices.size()/3);	
		mesh.F.resize(3, shapes[0].mesh.indices.size()/3);	

		for (int i = 0; i < (int)shapes[0].mesh.indices.size(); i++)
			mesh.F(i) = shapes[0].mesh.indices[i].vertex_index;
		for (int i = 0; i < (int)attrib.vertices.size(); i++)
			mesh.V(i) = attrib.vertices[i];
		

		float xmin = 1e9, xmax = 1e-9, ymin = 1e9, ymax = 1e-9, zmin = 1e9, zmax = 1e-9;
		for (int i = 0; i < mesh.V.cols(); i++) {
			xmin = mesh.V(0, i) < xmin ? mesh.V(0, i) : xmin;
			ymin = mesh.V(1, i) < ymin ? mesh.V(1, i) : ymin;
			zmin = mesh.V(2, i) < zmin ? mesh.V(2, i) : zmin;
			xmax = mesh.V(0, i) > xmax ? mesh.V(0, i) : xmax;
			ymax = mesh.V(1, i) > ymax ? mesh.V(1, i) : ymax;
			zmax = mesh.V(2, i) > zmax ? mesh.V(2, i) : zmax;
		}

		float dx = (xmax - xmin)*1.01;
		float dy = (ymax - ymin)*1.01;
		float dz = (zmax - zmin)*1.01;
		float dmax = std::max(std::max(dx, dy), dz);

		for (int i = 0; i < mesh.V.cols(); i++) {
			mesh.V(0, i) = (mesh.V(0, i) - xmin)/dmax;
			mesh.V(1, i) = (mesh.V(1, i) - ymin)/dmax;
			mesh.V(2, i) = (mesh.V(2, i) - zmin)/dmax;
		}
		
	}
	
	void init_render_vao() {
		model_matrix.setIdentity();
		float ar = (float)(WINDOW_WIDTH)/WINDOW_HEIGHT;
		Eigen::Vector3f eye(0, 0, -1);
		Eigen::Vector3f up(0, 1, 0);
		Eigen::Vector3f lookat(0, 0, 0);
		view_matrix = oglh::make_view_matrix<Eigen::Vector3f>(eye, lookat, up);	
		projection_matrix = oglh::make_perspective_matrix<float>(60.0f, ar, 0.3f, 50.0f); 	
		//projection_matrix = oglh::make_ortho_matrix<float>(-1, 1, -1, 1, 0.3, 50); 	
		
        std::string dir = "/home/amon/grive/development/Voxelization/src/glsl/";
		GLuint vs = 0, fs = 0;
		oglh::load_shader(vs, dir + "/PolygonVS.glsl", GL_VERTEX_SHADER);
		oglh::load_shader(fs, dir + "/PolygonFS.glsl", GL_FRAGMENT_SHADER);
		GLuint shaders[2] = {vs, fs};
		oglh::create_program(vao_render.program, shaders, 2);
        glUseProgram(vao_render.program);
        glGenVertexArrays(1, &vao_render.id_vao);
        glBindVertexArray(vao_render.id_vao);
		
        // -> vertices (buffer object)
		const GLfloat vertices[] = {
			-1.0f,-1.0f,-1.0f, -1.0f,-1.0f, 1.0f, -1.0f, 1.0f, 1.0f,  // Left Side
			-1.0f,-1.0f,-1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f,-1.0f,  // Left Side
			 1.0f, 1.0f,-1.0f, -1.0f,-1.0f,-1.0f, -1.0f, 1.0f,-1.0f,  // Back Side
			 1.0f, 1.0f,-1.0f,  1.0f,-1.0f,-1.0f, -1.0f,-1.0f,-1.0f,  // Back Side
			 1.0f,-1.0f, 1.0f, -1.0f,-1.0f,-1.0f,  1.0f,-1.0f,-1.0f,  // Bottom Side
			 1.0f,-1.0f, 1.0f, -1.0f,-1.0f, 1.0f, -1.0f,-1.0f,-1.0f,  // Bottom Side
			-1.0f, 1.0f, 1.0f, -1.0f,-1.0f, 1.0f,  1.0f,-1.0f, 1.0f,  // Front Side
			 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f,  1.0f,-1.0f, 1.0f,  // Front Side
			 1.0f, 1.0f, 1.0f,  1.0f,-1.0f,-1.0f,  1.0f, 1.0f,-1.0f,  // Right Side
			 1.0f,-1.0f,-1.0f,  1.0f, 1.0f, 1.0f,  1.0f,-1.0f, 1.0f,  // Right Side
			 1.0f, 1.0f, 1.0f,  1.0f, 1.0f,-1.0f, -1.0f, 1.0f,-1.0f,  // Top Side
			 1.0f, 1.0f, 1.0f, -1.0f, 1.0f,-1.0f, -1.0f, 1.0f, 1.0f,  // Top Side
		};
        glGenBuffers(1, &vao_render.id_vbo_vertex);
        glBindBuffer(GL_ARRAY_BUFFER, vao_render.id_vbo_vertex);
        glBufferData(GL_ARRAY_BUFFER, 12*3*3*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // <-

        // -> normals (buffer object) 
		const GLfloat normals[] = { 
			-1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f, // Left Side
			-1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f, // Left Side
			 0.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f, // Back Side
			 0.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f, // Back Side
			 0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f, // Bottom Side
			 0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f, // Bottom Side
			 0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f, // front Side
			 0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f, // front Side
			 1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f, // right Side
			 1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f, // right Side
			 0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, // top Side
			 0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, // top Side
		}; 
        glGenBuffers(1, &vao_render.id_vbo_normal);
        glBindBuffer(GL_ARRAY_BUFFER, vao_render.id_vbo_normal);
        glBufferData(GL_ARRAY_BUFFER, 12*3*3*sizeof(GLfloat), normals, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // <-
        
		// -> position (buffer object) 
        glGenBuffers(1, &vao_render.id_vbo_position);
        glBindBuffer(GL_ARRAY_BUFFER, vao_render.id_vbo_position);
        glBufferData(GL_ARRAY_BUFFER, n_size*3*sizeof(GLfloat), position.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // <-

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
	}

    void init_voxelization_vao() {
	
        std::string dir = "/home/amon/grive/development/Voxelization/src/glsl/";
		GLuint vs = 0, gs = 0, fs = 0;
		oglh::load_shader(vs, dir + "/VoxelizationVS.glsl", GL_VERTEX_SHADER);
		oglh::load_shader(gs, dir + "/VoxelizationGS.glsl", GL_GEOMETRY_SHADER);
		oglh::load_shader(fs, dir + "/VoxelizationFS.glsl", GL_FRAGMENT_SHADER);
		GLuint shaders[3] = {vs, gs, fs};
		oglh::create_program(vao_voxelization.program, shaders, 3);
        glUseProgram(vao_voxelization.program);
        glGenVertexArrays(1, &vao_voxelization.id_vao);
        glBindVertexArray(vao_voxelization.id_vao);
		
		// -> position (buffer object) 
        glGenBuffers(1, &vao_voxelization.id_vbo_position);
        glBindBuffer(GL_ARRAY_BUFFER, vao_voxelization.id_vbo_position);
        glBufferData(GL_ARRAY_BUFFER, mesh.F.cols()*3*sizeof(GLfloat), mesh.V.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // <-

    	// -> elements (buffer object) 
        glGenBuffers(1, &vao_voxelization.id_ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao_voxelization.id_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.F.cols()*3*sizeof(GLuint), mesh.F.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        // <-
		
	
		voxelResolution[0] = 23;
		voxelResolution[1] = 23;
		voxelResolution[2] = 23;
		scale = 0.4/voxelResolution[0];

		std::vector<uint8_t> image(voxelResolution[0]*voxelResolution[1]*voxelResolution[2], 0);
		//std::iota(image.begin(), image.end(), 1);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		// -> Occuancy Grid 
        glActiveTexture(GL_TEXTURE0);
        glGenTextures(1, &vao_voxelization.id_image_occupany);
        glBindTexture(GL_TEXTURE_3D, vao_voxelization.id_image_occupany);
        glTexStorage3D(GL_TEXTURE_3D, 1, GL_R8UI, voxelResolution[0], voxelResolution[1], voxelResolution[2]);
        glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, voxelResolution[0], voxelResolution[1], voxelResolution[2], GL_RED_INTEGER, GL_UNSIGNED_BYTE, image.data());
        glBindImageTexture(0, vao_voxelization.id_image_occupany, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R8UI);
		// <-
		
		//// -> Color Grid 
        //glActiveTexture(GL_TEXTURE1);
        //glGenTextures(1, &vao_voxelization.id_image_color);
        //glBindTexture(GL_TEXTURE_3D, vao_voxelization.id_image_color);
        //glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA8, voxelResolution[0], voxelResolution[1], voxelResolution[2]);
        ////glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width, image.height, GL_RGB, GL_UNSIGNED_BYTE, image.data.data());
        //glBindImageTexture(1, vao_voxelization.id_image_color, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
		//// <-
		
		voxelize();
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
		
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_3D, vao_voxelization.id_image_occupany);
		glGetTexImage(GL_TEXTURE_3D, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, image.data());
		glMemoryBarrier(GL_ALL_BARRIER_BITS);

		int sum = std::accumulate(image.begin(), image.end(), 0);

		n_size = 0;
		for (int i = 0; i < voxelResolution[2]; i++) {
			for (int j = 0; j < voxelResolution[1]; j++) {
				for (int k = 0; k < voxelResolution[0]; k++) {
					int index = i*voxelResolution[1]*voxelResolution[1] + j*voxelResolution[1] + k;
					if (image[index]) {
						position.push_back((float)k/voxelResolution[0]);	
						position.push_back((float)j/voxelResolution[1]);	
						position.push_back((float)i/voxelResolution[2]);	
						//printf("v: %d x: %d y: %d z: %d\n", image[index], k, j, i);
						//printf("v: %d i: %d\n", image[i_counter], i_counter);
						n_size++;
					}
				}
			}
		}
		std::cout << "n_size: " << n_size << std::endl;
    }
    
	void voxelize() {
        glUseProgram(vao_voxelization.program);
        glBindVertexArray(vao_voxelization.id_vao); 
		
        glBindBuffer(GL_ARRAY_BUFFER, vao_voxelization.id_vbo_position);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao_voxelization.id_ebo);
        glUniform3iv(glGetUniformLocation(vao_voxelization.program, "voxelResolution"), 1, voxelResolution);
		
		glDrawElements(GL_TRIANGLES, 3*mesh.F.cols(), GL_UNSIGNED_INT, 0);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glDisableVertexAttribArray(0);

    }
	
	void draw() {
        glUseProgram(vao_render.program);
        glBindVertexArray(vao_render.id_vao); 
		
		glVertexAttribDivisor(2, 1);

        glBindBuffer(GL_ARRAY_BUFFER, vao_render.id_vbo_vertex);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, vao_render.id_vbo_normal);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);
        
		glBindBuffer(GL_ARRAY_BUFFER, vao_render.id_vbo_position);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(2);
	 
        glUniform1f(glGetUniformLocation(vao_render.program, "scale"), scale);
        glUniformMatrix4fv(glGetUniformLocation(vao_render.program, "model_matrix"), 1, GL_FALSE, model_matrix.data());
        glUniformMatrix4fv(glGetUniformLocation(vao_render.program, "view_matrix"), 1, GL_FALSE, view_matrix.data());
        glUniformMatrix4fv(glGetUniformLocation(vao_render.program, "projection_matrix"), 1, GL_FALSE, projection_matrix.data());
		
		glDrawArraysInstanced(GL_TRIANGLES, 0, 12*3, n_size);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
	}

	
	void update_camera() {
		Eigen::Map<Eigen::Vector3f> eye(Camera::cam_position);
		Eigen::Map<Eigen::Vector3f> up(Camera::cam_up);
		Eigen::Map<Eigen::Vector3f> lookat(Camera::cam_lookat);
		view_matrix = oglh::make_view_matrix<Eigen::Vector3f>(eye, lookat, up);	

	}

    void advance(std::size_t iteration_counter, int64_t ms_per_frame) {
		update_camera();
        //double t = iteration_counter/100.0;
		//Eigen::Matrix3f rot1 = Eigen::AngleAxisf(t, Eigen::Vector3f::UnitY()).toRotationMatrix();

		//Eigen::Matrix4f rot = Eigen::Matrix4f::Identity();
		//rot.block(0, 0, 3, 3) = rot1;
        //model_matrix = rot;
    }

private:
	Eigen::Matrix4f model_matrix;
	Eigen::Matrix4f projection_matrix;
	Eigen::Matrix4f view_matrix;

    RenderVAO vao_render;
    VoxelizationVAO vao_voxelization;
	
	int n_size;
	std::vector<float> position;
	std::vector<float> color;
	float scale;
	int voxelResolution[3];	
	Mesh mesh;
};


int main() {
	GLFWwindow *window;
    oglh::init_gl("Voxelization", WINDOW_WIDTH, WINDOW_HEIGHT, &window);
	glfwSetCursorPosCallback(window, Camera::mousemove_glfwCursorPosCallback);
	glfwSetScrollCallback(window, Camera::mousemove_glfwScrollCallback);
	glfwSetMouseButtonCallback(window, Camera::mousemove_glfwMouseButtonCallback);
    //oglh::set_dummy_key_callback(window);
    ToyWorld world;
	world.init();
	
    std::chrono::high_resolution_clock clock;
    std::size_t iteration_counter = 0;
    std::chrono::high_resolution_clock::time_point t0 = clock.now();
    std::chrono::milliseconds mspf;
    while (oglh::is_window_alive(window)) {
        oglh::poll_events();
		oglh::clear_screen();

        world.advance(iteration_counter++, mspf.count());
        world.draw();
	
        mspf = std::chrono::duration_cast<std::chrono::milliseconds>(clock.now() - t0);
        t0 = clock.now();

        oglh::swap_window_buffer(window);
    }
	world.term();
    oglh::terminate_window();	

    return 0;
}
