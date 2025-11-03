#include "SDMTL.hpp"
#include "sys/stat.h"
#include "string_extended.hpp"
#include "log.h"
#include "SharedTypes.hpp" // for GLOBALS
constexpr float CART_WIDTH = 4.0f;
constexpr float INITIAL_SCALE = 1004478.250000;
constexpr simd::float2 INITIAL_OFFSET = {-0.748629, 0.080157};

void SDMTL::increment_offset(float incr_x, float incr_y, float scale) {
	simd::float2 offset = read_mtl_buf<simd::float2>(mtl.offset_buf);
	offset.x += (incr_x) / scale;
	offset.y += (incr_y) / scale;
//	printf("offset: %f %f\n", offset.x, offset.y);
	write_mtl_buf(offset, mtl.offset_buf);
}
void SDMTL::increment_scale(float incr) {
	float scale = read_mtl_buf<float>(mtl.scale_buf);
	if (incr > 0) scale *= incr;
	if (incr < 0) scale /= -incr;
//	printf("scale : %f\n", scale);
	write_mtl_buf(scale, mtl.scale_buf);
}


MTL::Buffer *SDMTL::init_fullscreen_buf() {
	// two triangles which cover the screen
	simd::float4 verticies[] = {
		{  -1.0, -1.0, 0.0, 1.0},
		{   1.0, -1.0, 0.0, 1.0},
		{  -1.0,  1.0, 0.0, 1.0},
		{  -1.0,  1.0, 0.0, 1.0},
		{   1.0, -1.0, 0.0, 1.0},
		{   1.0,  1.0, 0.0, 1.0},
	};
	return mtl.device->newBuffer(&verticies, sizeof(verticies), MTL::ResourceStorageModeShared);

}

MTL::Buffer *SDMTL::init_offset_buf() {
	simd::float2 offset = INITIAL_OFFSET;
	return mtl.device->newBuffer(&offset, sizeof(offset), MTL::ResourceStorageModeShared);
}
MTL::Buffer *SDMTL::init_scale_buf() {
	float scale = INITIAL_SCALE;
	return mtl.device->newBuffer(&scale, sizeof(scale), MTL::ResourceStorageModeShared);
}

MTL::Buffer *SDMTL::init_framecount_buf() {
	uint32_t framecount = 0;
	return mtl.device->newBuffer(&framecount, sizeof(framecount), MTL::ResourceStorageModeShared);
}

MTL::Buffer *SDMTL::init_globals_buf() {
	float aspect_ratio = (float)sdl.width_px / sdl.height_px;
	GLOBALS G = {
		.viewport_width = (float)sdl.width_px,
		.viewport_height = (float)sdl.height_px,
		.cartesian_width = CART_WIDTH,
		.cartesian_height = (float)CART_WIDTH / aspect_ratio,
	};
	return mtl.device->newBuffer(&G, sizeof(G), MTL::ResourceStorageModeShared);

}

void SDMTL::sdl_init() {
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");
	if (SDL_Init(SDL_INIT_EVERYTHING)) {
		logsdl("Failed to initialize SDL");
		logexit(EXIT_FAILURE);
	}
	int render_flags = 0;
	int window_flags = SDL_WINDOW_SHOWN & SDL_WINDOW_METAL & SDL_WINDOW_INPUT_FOCUS;

	sdl.win = SDL_CreateWindow("Metal hello world", 0, 0, sdl.width_px, sdl.height_px, window_flags);
	if (!sdl.win) logsdl_exit("Failed to initialize window");

	sdl.renderer = SDL_CreateRenderer(sdl.win, -1, render_flags);
	if (!sdl.renderer) logsdl_exit("Failed to initialize renderer");

}
static inline void mtl_perror(NS::Error * err) {
	char *curr_error_start = (char*)err->localizedDescription()->utf8String();
	log("%s\n", curr_error_start);
}

static inline void mtl_perror(NS::Error * err, const char* msl_path) {
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

MTL::Library *SDMTL::compile_msl_lib(const char* msl_path) {
	char *msl_src = read_file(msl_path);
	if (!msl_src) {
		logsdl("Failed to read file '%s' \n", msl_path);
		logexit(EXIT_FAILURE);
	}
	auto msl_src_ascii = NS::String::string(msl_src, NS::ASCIIStringEncoding);

	auto compile_opts = MTL::CompileOptions::alloc()->init();
	MTL::Library* lib = mtl.device->newLibrary(msl_src_ascii, compile_opts, &err);
	if (!lib) {
		mtl_perror(err, msl_path);
		logexit(EXIT_FAILURE);
	}
	free(msl_src);
	compile_opts->release();
	return lib;
}

MTL::Function *SDMTL::setup_vertex_fn(MTL::Library* lib, MTL::RenderPipelineDescriptor* pld, const char* vertex_fn_name_cstr) {
	auto vertex_fn_name = NS::String::string(vertex_fn_name_cstr, NS::ASCIIStringEncoding);
	auto vertex_fn = lib->newFunction(vertex_fn_name);
	pld->setVertexFunction(vertex_fn);
	return vertex_fn;

}

MTL::Function *SDMTL::get_fragment_fn(MTL::Library* lib, MTL::RenderPipelineDescriptor* pld, const char* fragment_fn_name_cstr) {
	auto fragment_fn_name = NS::String::string(fragment_fn_name_cstr, NS::ASCIIStringEncoding);
	auto fragment_fn = lib->newFunction(fragment_fn_name);
	pld->setFragmentFunction(fragment_fn);
	return fragment_fn;

}
ssize_t syscall_file_size(const char* filename) {
	struct stat s;
	stat(filename, &s);
	return s.st_size;
}

char *read_file(const char* filename) {
	size_t const MAX_FILE_SZ = 50'000'000; // 50MB
	size_t file_size = syscall_file_size(filename);

	FILE * file_ptr = fopen(filename, "rb");

	if (!file_ptr) {
		logfatalerrno("Unable to open file '%s'.\n", filename);
		return NULL;
	}
	if (file_size == 0) {
		fclose(file_ptr);
		logfatal("file '%s' is empty!\n", filename);
		return NULL;
	}


	if (file_size > MAX_FILE_SZ) {
		logfatal("Requested file '%s' is too large! (%zu>%zu)\n", filename, file_size, MAX_FILE_SZ);
		fclose(file_ptr);
		return NULL;
	}
	char *file_contents = (char*)calloc(file_size + 1, sizeof(char));
	if (!file_contents) {
		logfatal("Unable to alloc buffer for file '%s'.\n", filename);
		fclose(file_ptr);
		return NULL;
	}

	int n_read = fread(file_contents, file_size, 1, file_ptr);
	if (n_read != 1) {
		logfatal("Unable to read file contents for file '%s'.\n", filename);
		free(file_contents);
		fclose(file_ptr);
		return NULL;
	}
	fclose(file_ptr);
	return file_contents;
}
void SDMTL::handle_input() {
	SDL_Event event;
	while( SDL_PollEvent( &event ) ) {
		switch( event.type ) {
		case SDL_QUIT:
			sdl.running = false;
		case SDL_KEYDOWN:
			handle_keypress(event.key.keysym);
			break;

		case SDL_KEYUP:
			break;

		default:
			break;
		}
	}
}

const float incr_offset = 0.1f;
const float incr_scale = 1.1f;
// *INDENT-OFF*
void SDMTL::handle_keypress(SDL_Keysym keysym) {
	float scale = read_mtl_buf<float>(mtl.scale_buf);
	switch (keysym.sym) {
	// exiting
	case SDLK_ESCAPE: 	sdl.running = false; break;
	case 'c':
	case 'C': 		if (keysym.mod & KMOD_CTRL){ sdl.running = false;} break;

	// zoom
	case 'w': increment_scale(incr_scale); break;
	case 's': increment_scale(-incr_scale); break;

	// movement
	case SDLK_RIGHT: 	increment_offset(incr_offset, 0, scale); break;
	case SDLK_LEFT: 	increment_offset(-incr_offset, 0, scale); break; 
	case SDLK_UP: 		increment_offset(0, incr_offset, scale); break;
	case SDLK_DOWN: 	increment_offset(0, -incr_offset, scale); break;
	}
}
