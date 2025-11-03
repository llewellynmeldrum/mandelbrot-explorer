#pragma once


#include "SDL_pixels.h"
#include "SDL_render.h"
#include <Metal/Metal.hpp>
#include <QuartzCore/CAMetalLayer.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <simd/simd.h>

// windowing, input
#include <SDL.h>
#include <SDL_ttf.h>
#include "log.h"

ssize_t syscall_file_size(const char* filename);
char *read_file(const char* filename);


struct SDL_Context {
	SDL_Window *win = nullptr;
	SDL_Renderer *renderer = nullptr;
	SDL_MetalView metal_view = nullptr;
	bool running = true;
	int width_px =  800;
	int height_px = 800;
	int win_flags = SDL_WINDOW_METAL & SDL_WINDOW_INPUT_FOCUS;
	int renderer_flags = SDL_RENDERER_PRESENTVSYNC & SDL_RENDERER_ACCELERATED;
	SDL_PixelFormatEnum pixel_fmt = SDL_PIXELFORMAT_BGRA8888;
	size_t bytes_per_pixel = 4;
	TTF_Font *ui_font;
	std::string ui_text;
	SDL_Surface *ui_s_bgra;
};

constexpr float A_OPAQUE = 1.0f;
constexpr float A_CLEAR = 0.0f;

// forgive my chud C code i cant think of a c++ way to do this without annoying templates
#define RGB(r,g,b) {r/255.0f, g/255.0f, b/255.0f, A_OPAQUE}
#define RGBA(r,g,b,a) {r/255.0f, g/255.0f, b/255.0f, a}

#define COL_MAGENTA 	RGB(255, 0, 255)
#define COL_BLACK 	RGB(0, 0, 0)
#define COL_WHITE 	RGB(255, 255, 255)
// metal state
struct MTL_Context {
	CA::MetalLayer *layer;
	MTL::Device *device;
	MTL::CommandQueue *command_queue;
	CA::MetalDrawable *drawable;
	MTL::RenderPipelineState *render_pipeline_state;
	MTL::ComputePipelineState *compute_pipeline_state;
	MTL::Buffer *vertex_buf;
	MTL::Buffer *globals_buf;
	MTL::Buffer *framecount_buf;
	MTL::Buffer *offset_buf;
	MTL::Buffer *scale_buf;

	MTL::SamplerState *ui_sampler;
	MTL::Texture *ui_tex;

	NS::UInteger vtx_offset = 0;
	NS::UInteger vtx_count = 6;
	MTL::PrimitiveType primitive_type = MTL::PrimitiveTypeTriangle;

	MTL::ClearColor clear_color = COL_MAGENTA;
	simd:: float4 triangle_color = COL_BLACK;

	static constexpr MTL::PixelFormat pixel_fmt = MTL::PixelFormatBGRA8Unorm;
	static constexpr NS::UInteger bytes_per_pixel = 4;
};
// *INDENT-OFF*
class SDMTL {
  public:
	template <typename T>
	void write_mtl_buf(T val, MTL::Buffer * mtl_buf) {
		memcpy(mtl_buf->contents(), &val, sizeof(T));
	}

	template <typename T>
	T read_mtl_buf(MTL::Buffer * mtl_buf) {
		T val;
		memcpy(&val, mtl_buf->contents(), sizeof(T));
		return val;
	}
	void init(const char* mtl_shader_path);
	void run();
	void draw_ui();
	void cleanup();

	size_t generation = 0;
	void reset_sim();
	void restart_sim(float living_ratio);

	SDL_Context sdl;
	MTL_Context mtl;

MTL::SamplerState *init_linear_sampler(MTL::Device* dev);
  private:
	MTL::Library* compile_msl_lib(const char* msl_path);
	void handle_input();
	void handle_keypress(SDL_Keysym key);
	void mtl_init(const char* shader_path);
	void sdl_init();
	void update();
	void draw();

	NS::Error *err;

	MTL::ComputePipelineState *setup_compute_pipeline(MTL::Library* lib, MTL::RenderPipelineDescriptor* pld, const char* compute_fn_name_cstr);
	MTL::Function *setup_vertex_fn(MTL::Library* lib, MTL::RenderPipelineDescriptor* pld, const char* vertex_fn_name_cstr);
	MTL::Function *get_fragment_fn(MTL::Library* lib, MTL::RenderPipelineDescriptor* pld, const char* fragment_fn_name_cstr);
	MTL::Buffer *init_fullscreen_buf();
	void increment_offset(float incr_x, float incr_y, float scale);
	void increment_scale(float incr);
	MTL::Buffer *init_globals_buf();
	MTL::Buffer *init_framecount_buf();
	MTL::Buffer *init_offset_buf();
	MTL::Buffer *init_scale_buf();
	// metal
	void create_render_pipeline();

	void encode_render_command(MTL::RenderCommandEncoder* render_encoder);
static inline void mtl_perror(NS::Error * err) {
	char *curr_error_start = (char*)err->localizedDescription()->utf8String();
	log( SET_BOLD "%s\n", curr_error_start);
}

static inline void mtl_perror(NS::Error * err, const char* msl_path) {
		mtl_perror(err);
}


static inline void _mtl_perror(NS::Error * err, const char* msl_path) {
	bool errors_finished = false;
	char *curr_error_start = (char*)err->localizedDescription()->utf8String();
	char *nxt_error_start = NULL;
	while (!errors_finished) {
		if (nxt_error_start) {
			curr_error_start = nxt_error_start;
		}
		char *line_num = strchr(curr_error_start, ':') + 1;
		char *colon_2 = strchr(line_num, ':') + 1;
		char *error_title = strchr(colon_2, ':') + 2;
		char *error_desc = strchr(error_title, ':') + 2;
		char *space_after_colon_1 = strchr(line_num, ' ');
		*space_after_colon_1 = '\0';
		char *space_after_colon_3 = strchr(error_title, ' ');
		*space_after_colon_3 = '\0';
		char *newl = strchr(error_desc, '\n');
		if (!newl) break;
		*newl = '\0';

		char *src_ref = newl + 1;
		nxt_error_start = strstr(src_ref, "program_source");
		if (nxt_error_start) {
			char *end = nxt_error_start - 1;
			*end = '\0';

		} else {
			errors_finished = true;
		}
//			dprintbuf("ERROR:", curr_error_start, strlen(curr_error_start), 0);
		log(SET_BOLD "%s:%s " SET_RED "%s\n" SET_WHITE "%s\n" SET_NOBOLD "%s" SET_CLEAR "\n\n",
		    msl_path, line_num, error_title, error_desc, src_ref);

	}
}


};

