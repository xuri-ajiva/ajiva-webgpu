struct UniformData {
    projectionMatrix: mat4x4f,
    viewMatrix: mat4x4f,
    worldPos: vec3f,
    time: f32,
};

struct VertexOutput {
    @builtin(position) position: vec4f
};

//@group(0) @binding(0) var<uniform> u: UniformData;
@group(0) @binding(0) var<storage,read> outputBuffer: array<f32>;

@vertex
fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> VertexOutput {
    var p = vec2f(0.0, 0.0);
    if (in_vertex_index == 0u) {
        p = vec2f(-0.5, -0.5);
    } else if (in_vertex_index == 1u) {
        p = vec2f(0.5, -0.5);
    } else {
        p = vec2f(0.0, 0.5);
    }

    var out: VertexOutput;
    out.position = /*u.projectionMatrix * u.viewMatrix **/ vec4f(p, 0.0, 1.0);
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    return vec4f(1.0, 0.0, 0.0, 1.0);
}