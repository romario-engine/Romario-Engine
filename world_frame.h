#pragma once
#include "graphics/graphics.h"
#include "global/time.h"
#include "graphics/world_object.h"
#include "graphics/heightmap.h"
#include "ui/ui.h"
#include "ui/window.h"
#include "ui/text_field.h"
#include "ui/flow_layout.h"
#include "ui/button.h"
#include "ui/grid_layout.h"
#include "world_frame.h"
#include "ui/image_view.h"
#include "ui/option_list.h"
#include "global/timer.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

using namespace DirectX;

enum struct texture_mode : uint32
{
	normal,
	landscape,
	pbr
};

#pragma pack(push, 1)
struct constant_buffer
{
	XMMATRIX wvp;
	XMMATRIX world;
	uint32 width;
	uint32 height;
	uint32 animated;
	uint32 radius;
	vector<float32, 4> color;
	uint32 manual_uv;
	float32 begin_x;
	float32 begin_z;
	float32 size_x;
	float32 size_z;
	texture_mode tm;
	uint32 landscape_interpolation;
	uint32 padding1;

	vector<float32, 3> light_dir;
	uint32 padding3;
	vector<float32, 3> light_color;
	uint32 padding4;
	
    vector<float32, 3> camera;
	uint32 padding5;
    vector<float32, 4> baseColorFactor;
	float32 metallic_factor;
    float32 roughness_factor;
};
#pragma pack(pop)

struct constant_buffer_joints
{
	XMMATRIX final_transform[250];
};

struct camera_view
{
	XMVECTOR position;
	XMVECTOR target;
	XMVECTOR up;
	XMMATRIX projection;
	XMMATRIX view;
};

struct camera_movement
{
	XMVECTOR default_forward;
	XMVECTOR default_right;
	XMVECTOR camera_forward;
	XMVECTOR camera_right;
	float32 camera_yaw;
	float32 camera_pitch;
	bool move_forward;
	bool move_back;
	bool move_right;
	bool move_left;
	timestamp last_move_time;
};

struct world_object_selection
{
	bool selected;
	world_object outline;
	ID3DBlob *hit_test_vertex_shader_buffer;
	ID3D11VertexShader *hit_test_vertex_shader;
	ID3DBlob *hit_test_pixel_shader_buffer;
	ID3D11PixelShader *hit_test_pixel_shader;
	ID3DBlob *outline_vertex_shader_buffer;
	ID3D11VertexShader *outline_vertex_shader;
	ID3DBlob *outline_pixel_shader_buffer;
	ID3D11PixelShader *outline_pixel_shader;
};

struct skymap
{
	ID3D10Blob *vertex_shader_buffer;
	ID3D10Blob *pixel_shader_buffer;
	ID3D11VertexShader *vertex_shader;
	ID3D11PixelShader *pixel_shader;
	texture_view sky_tv;
	ID3D11DepthStencilState *depth_stencil;
	world_object sphere;
	XMMATRIX sphereWorld;
};

struct world
{
	window *wnd;
	uint32 width;
	uint32 height;
	IDXGISwapChain *swap_chain;
	ID3D11Device *device;
	ID3D11DeviceContext *device_context;
	ID3D11RenderTargetView *render_target_view;
	ID3D11DepthStencilView *depth_stencil_view;
	ID3D11Texture2D *depth_stencil_buffer;
	constant_buffer cb;
	ID3D11Buffer *cb_handler;
	constant_buffer_joints cb_joints;
	ID3D11Buffer *cb_joints_handler;
	ID3D11BlendState *blending;

	ID3DBlob *vertex_shader_buffer;
	ID3D11VertexShader *vertex_shader;
	ID3DBlob *pixel_shader_buffer;
	ID3D11PixelShader *pixel_shader;
	ID3D11InputLayout *vertex_layout;
	ID3D11SamplerState *sampler_state;

	ID3D11Texture2D *render_target;
	ID3D11RenderTargetView *rt_view;
	ID3D11ShaderResourceView *rt_shader_resource_view;

	ID3DBlob *ui_vertex_shader_buffer;
	ID3D11VertexShader *ui_vertex_shader;
	ID3DBlob *ui_pixel_shader_buffer;
	ID3D11PixelShader *ui_pixel_shader;
	ID3D11Texture2D *ui_texture;
	ID3D11ShaderResourceView *ui_srv;
	world_object uiwo;

	camera_view camera;
	camera_movement movement;
	world_object_selection selection;
	float32 render_distance;

	array<world_model> models;
	world_object cylinder;
	world_object cube;
	texture_view cube_tv;
	skymap sky;
	heightmap hm;
	ID3D11ShaderResourceView *assigned_textures[9];

	uint32 fps;
	timestamp fps_second_accumulative;
	uint32 fps_accumulative;
	
	frame *layout;
	grid_layout *main_layout;
	flow_layout *pick_layout;
	text_field *fps_frame;
	radio_button_group picked_texture;
	bitmap snow_texture_bmp;
	button *snow_texture_button;
	bitmap spring_texture_bmp;
	button *spring_texture_button;
	bitmap summer_texture_bmp;
	button *summer_texture_button;
	bitmap autumn_texture_bmp;
	button *autumn_texture_button;

	texture_view snow_texture;
	texture_view spring_texture;
	texture_view summer_texture;
	texture_view autumn_texture;

	flow_layout *menu_layout;
	option_list *menu1;
	text_field *menu1_text;
	button *pick_option1;
	text_field *pick_option1_text;
	button *pick_option2;
	text_field *pick_option2_text;
	button *pick_option3;
	text_field *pick_option3_text;

	world();
	~world();
	void initialize_common_resources();
	void initialize_ui_resources();
	void initialize_outline_resources();
	void initialize_sky_resources();
	void release_common_resources();
	void release_ui_resources();
	void release_outline_resources();
	void release_sky_resources();
	void initialize_scene();
	void find_pick_ray(XMVECTOR *position, XMVECTOR *direction);
	bool point_in_triangle(XMVECTOR point, XMVECTOR v1, XMVECTOR v2, XMVECTOR v3);
	bool ray_intersects_triangle(
		XMVECTOR position, XMVECTOR direction,
		XMVECTOR v1, XMVECTOR v2, XMVECTOR v3, XMMATRIX world_transform, float32 *distance);
	void mouse_click();
	void update_movement();
	void update_fps();
	void resize();
	void assign_texture(uint32 slot, uint32 map_idx, ID3D11ShaderResourceView *srv);
	void render_heightmap();
	void render_sky();
	void render_objects();
	void render_outline();
	void render_ui(frame *fm, graphics_displayer *gd, bitmap *bmp);
	void render(frame *fm, graphics_displayer *gd, bitmap *bmp);
};

struct world_frame
{
	frame fm;
	world wld;

	world_frame();
};
