struct VertexInput {
	@location(0) position: vec2f,
	@location(1) color: vec3f,
};

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) color: vec3f,
};

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
    let offset = vec2f(-0.6875, -0.463);
    let aspect = 800.0 / 600.0;
    var out: VertexOutput;
    out.position = vec4f(in.position * vec2f(1.0, aspect) + offset, 0.0, 1.0);
    out.color = in.color; // forward to the fragment shader
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    return vec4f(in.color, 1.0);
}