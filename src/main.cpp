// Metal
#include "SDL_metal.h"
#include <unistd.h>
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define NS_PRIVATE_IMPLEMENTATION
#include "SDMTL.hpp"
#include "SharedTypes.hpp" // for GLOBALS

#include <iostream>
#include <functional>
#include <cstdlib>

#include "log.h"
#include <sys/stat.h>
#include <iomanip>

#include "ansi_colors.hpp"
constexpr const char *CONTROLS_STR = \
                                     "[W/S] 	- Zoom in/out (exponential)\n"\
                                     "[↑/↓/←/→] - Move around (relative to zoom)";


int main() {
	SDMTL sim = SDMTL();
	sim.init("shaders/mandelbrot.metal");
	sim.run();
	sim.cleanup();
	exit(EXIT_SUCCESS);
}


// *INDENT-OFF*
void SDMTL::init(const char* shader_src_path) {
	sdl_init();
	auto init_pool = NS::AutoreleasePool::alloc()->init();
	{
		mtl_init(shader_src_path);
	}
	init_pool->release();
	std::cout << ansi::inverse << ansi::bold << std::setw(40) << std::left << "CONTROLS:" << ansi::reset	<<
	std::endl << ansi::bold << "    [W/S]" << ansi::reset << " Zoom in/out (exponential)" 			<< 
	std::endl << ansi::bold << "[↑/↓/←/→]" << ansi::reset << " Move around (relative to zoom level)\n"	<<
	std::endl;

}
// *INDENT-ON*

void SDMTL::mtl_init(const char* shader_src_path) {

	mtl.layer = (CA::MetalLayer*)SDL_RenderGetMetalLayer(sdl.renderer);
	mtl.device = mtl.layer->device();					/* MUST BE RELEASED LATER */
	mtl.command_queue = mtl.device->newCommandQueue();			/* MUST BE RELEASED */

	auto lib = compile_msl_lib(shader_src_path);

	auto pipeline_descriptor = MTL::RenderPipelineDescriptor::alloc()->init();
	if (!pipeline_descriptor) {
		logfatal("Failed to create pipeline!\n");
		logexit(EXIT_FAILURE);
	}
	mtl.vertex_buf = init_fullscreen_buf();
	mtl.globals_buf = init_globals_buf();
	mtl.framecount_buf = init_framecount_buf();
	mtl.offset_buf = init_offset_buf();
	mtl.scale_buf = init_scale_buf();

	auto vert_fn = setup_vertex_fn(lib, pipeline_descriptor, "vertex_shader");
	auto frag_fn = get_fragment_fn(lib, pipeline_descriptor, "fragment_shader");


	auto pixel_format = (MTL::PixelFormat)mtl.layer->pixelFormat();
	pipeline_descriptor->colorAttachments()->object(0)->setPixelFormat(pixel_format);
	mtl.render_pipeline_state = mtl.device->newRenderPipelineState(pipeline_descriptor, &err);
	if (!mtl.render_pipeline_state) {
		logfatal("Failed to create pipeline state!\n");
		mtl_perror(err);
		logexit(EXIT_FAILURE);
	}


	pipeline_descriptor->release();
	vert_fn->release();
	frag_fn->release();
	lib->release();
}




void SDMTL::run() {
	while(sdl.running) {
		handle_input();
		auto draw_pool = NS::AutoreleasePool::alloc()->init();
		draw();
		draw_pool->release();
	}
}

void SDMTL::draw() {
	/* init command buffer from command queue */
	mtl.drawable = mtl.layer->nextDrawable();
	auto buffer = mtl.command_queue->commandBuffer();

	/* create render pass */
	auto render_pass = MTL::RenderPassDescriptor::alloc()->init();
	auto CAD = render_pass->colorAttachments()->object(0);
	CAD->setTexture(mtl.drawable->texture());
	CAD->setLoadAction(MTL::LoadActionClear);
	CAD->setStoreAction(MTL::StoreActionStore);
	CAD->setClearColor(mtl.clear_color);



	uint32_t framecount_incr = 1 + read_mtl_buf<uint32_t>(mtl.framecount_buf);
	write_mtl_buf(framecount_incr, mtl.framecount_buf);



	// x = red
	// y = green
	/* command encoding */
	auto command_encoder = buffer->renderCommandEncoder(render_pass);
	render_pass->release();
	command_encoder->setRenderPipelineState(mtl.render_pipeline_state);
	command_encoder->setVertexBuffer(mtl.globals_buf, 0, 0);
	command_encoder->setVertexBuffer(mtl.vertex_buf, 0, 1);

	command_encoder->setFragmentBuffer(mtl.globals_buf, 0, 0);
	command_encoder->setFragmentBuffer(mtl.framecount_buf, 0, 1);
	command_encoder->setFragmentBuffer(mtl.offset_buf, 0, 2);
	command_encoder->setFragmentBuffer(mtl.scale_buf, 0, 3);

	command_encoder->drawPrimitives(mtl.primitive_type, mtl.vtx_offset, mtl.vtx_count);
	command_encoder->endEncoding();


	buffer->presentDrawable(mtl.drawable);
	buffer->commit();
	buffer->waitUntilCompleted();

	// update framecount

//	command_buf->release();
}


/* private */


void SDMTL::cleanup() {
	SDL_DestroyRenderer(sdl.renderer);
	SDL_DestroyWindow(sdl.win);
	SDL_Quit();
	mtl.device->release();
}



