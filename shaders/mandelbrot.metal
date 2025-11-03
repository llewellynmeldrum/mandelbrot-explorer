#include <metal_stdlib>

#include "../include/SharedTypes.hpp" /*
struct GLOBALS {
	const float viewport_width;
	const float viewport_height;
	const float cartesian_width;
	const float cartesian_height;
}; */
using namespace metal;

// *INDENT-OFF*


struct VertexOut {
	/**/
	float4 view [[position]];
	float2 cart [[center_no_perspective]];
};
typedef float4 FragOut;

#define BLACK {0.0f,0.0f,0.0f,1.0f}
#define RED {1.0f,0.0f,0.0f,1.0f}
#define GREEN {0.0f,1.0f,0.0f,1.0f}
#define BLUE {0.0f,0.0f,1.0f,1.0f}
#define WHITE {1.0f,1.0f,1.0f,1.0f}
#define GREY {0.8f,0.8f,0.8f,1.0f}
#define LIGREY {0.4f,0.4f,0.4f,1.0f}
#define LILIGREY {0.25f,0.25f,0.25f,1.0f}

static inline float2 clip_to_cartesian(float4 clip, constant GLOBALS& G){
	float2 ndc = clip.xy / clip.w; // normalized device coords
	float2 half_span = {
		G.cartesian_width * 0.5f,
		G.cartesian_height * 0.5f,
	};
	return ndc * half_span;

}


vertex
VertexOut vertex_shader(constant GLOBALS& G[[buffer(0)]],
			constant float4* vertex_buf [[buffer(1)]],
              		uint id [[vertex_id]]) {
	float4 clip_pos = vertex_buf[id];


	float2 cart = clip_to_cartesian(clip_pos, G);



	return (VertexOut) {
		.view = clip_pos,
		.cart = cart,
	};
}


constant float view_line_thick = 10.1f;
constant float cart_line_thick= 0.1f;
constant float LINE_THICK = 0.01f;


#define PLOT_POINT(x,y, thickness, color) if (distance(float2{x,y}, in.cart.xy)<=thickness){ return color;}

struct ColorMask{
	float4 col;
	float mask;

	inline float4 get_mix(float4 col){
		return mix(col, this->col, this->mask);
	}

};
// given a point 'p', return 1 if on the grid lines, return 0 if not.
static inline float calc_gridline_mask(float2 p, float gap, float thick){
	float half_thick = thick*0.5f;
	float2 r = abs(fract(p/gap)-0.5f);
	float2 dist_to_nearest_line_xy = (0.5f -r ) * gap;
	float dist_to_nearest_line = min(dist_to_nearest_line_xy.x, dist_to_nearest_line_xy.y);
	return (dist_to_nearest_line<=thick ? 1.0f: 0.0f);

}
// 
static inline float4 mandelbrot_colormap(float iterations, float max_iterations){
	const float4 bg_col = WHITE;
	if (iterations==max_iterations) return bg_col; // didnt escape 
	if (iterations<=max_iterations*0.1f) return float4(0.0f,0.0f,0.0f,1.0f); // escaped, easily
	if (iterations<=max_iterations-(max_iterations*0.1f)) return float4(0.5f,0.5f,0.5f,1.0f); // escaped, barely

}

static inline float4 mandelbrot_color(float2 p, uint32_t max_iterations, float2 offset, float scale = 1.0f){
	p-= offset;
	p/=scale;
	p+= offset;
	float x0 = p.x;
	float y0 = p.y;

	float x = 0.0f;
	float y = 0.0f;
	uint32_t iteration = 0;
	while ((x*x + y*y <= 4.0f) && iteration< max_iterations){
		float xtemp = x*x - y*y + x0;
		y = 2.0f*x*y + y0;
		x = xtemp;
		++iteration;
	}
	// if it escapes (dist(xy, center)>2), then 0.0f, else 1.0f
	return mandelbrot_colormap(iteration,max_iterations);
}

float scale_exp(uint32_t framecount){
	float s = fmax(0.0f, (float)framecount);
	return s*s;
}

fragment
float4 fragment_shader( VertexOut in[[stage_in]], 
			constant GLOBALS& G [[buffer(0)]],
			constant uint32_t& framecount [[buffer(1)]],
		       	constant float2& offset[[buffer(2)]],
		       	constant float& scale[[buffer(3)]]){
	const uint32_t max_iterations = 1000;
//	float scale = scale_exp(framecount);
	float4 col = mandelbrot_color(in.cart,max_iterations, offset,scale);
	return col;
}



