
struct UniformData {
    color: vec4f,
    time: f32,
    aspect: f32,
};

struct VertexInput {
	@location(0) position: vec2f,
	@location(1) color: vec3f,
};

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) color: vec3f,
};

@group(0) @binding(0) var<uniform> u: UniformData;

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
    var offset = vec2f(-0.6875, -0.463);
	offset += 0.3 * vec2f(cos(u.time), sin(u.time));
    var out: VertexOutput;
    out.position = vec4f(in.position * vec2f(1.0, u.aspect) + offset, 0.0, 1.0);
    out.color = in.color; // forward to the fragment shader
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    return vec4f(in.color * u.color.rgb, 1.0);
}