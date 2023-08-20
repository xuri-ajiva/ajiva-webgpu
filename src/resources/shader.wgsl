
struct UniformData {
    projectionMatrix: mat4x4f,
    viewMatrix: mat4x4f,
    modelMatrix: mat4x4f,
    color: vec4f,
    time: f32
};

struct Light {
    position: vec4f,
    color: vec4f
};
struct LightningUniform{
    lights: array<Light, 4>,
    ambient: vec4f
};


struct VertexInput {
	@location(0) position: vec3f,
	@location(1) normal: vec3f,
	@location(2) color: vec3f,
    @location(3) uv: vec2f,
};

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) color: vec3f,
    @location(1) normal: vec3f,
    @location(2) uv: vec2f,
    @location(3) worldPosition: vec3f
};

@group(0) @binding(0) var<uniform> u: UniformData;
@group(0) @binding(1) var textureSampler: sampler;
@group(0) @binding(2) var gradientTexture: texture_2d<f32>;
@group(0) @binding(3) var<uniform> l: LightningUniform;

const pi = 3.14159265359;

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    let mvp = u.projectionMatrix * u.viewMatrix * u.modelMatrix;
	out.position = mvp * vec4f(in.position, 1.0);
    out.color = in.color;
    out.normal = (u.modelMatrix * vec4f(in.normal, 0.0)).xyz;
    out.uv = in.uv;
    out.worldPosition = (u.modelMatrix * vec4f(in.position, 1.0)).xyz;
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
	let normal = normalize(in.normal);

/*	let lightColor1 = vec3f(1.0, 0.9, 0.6);
	let lightColor2 = vec3f(0.6, 0.9, 1.0);
	let lightDirection1 = vec3f(0.5, -0.9, 0.1);
	let lightDirection2 = vec3f(0.2, 0.4, 0.3);
	let shading1 = max(0.0, dot(lightDirection1, normal));
	let shading2 = max(0.0, dot(lightDirection2, normal));
	let shading = shading1 * lightColor1 + shading2 * lightColor2;*/

	var shading = l.ambient.rgb;
	//point lights
	for (var i = 0; i < 4; i++) {
        let light = l.lights[i];
        //compute light pos relative to model
        let lightPos = (u.modelMatrix * light.position).xyz;
        let lightDir = normalize(lightPos - in.worldPosition);
        //compute falloff based on intensity (pos w) and distance
        let falloff = light.color.a / length(lightPos - in.worldPosition);
        //compute shading
        let shading1 = max(0.0, dot(lightDir, normal));
        shading += shading1 * light.color.rgb * falloff;
    }

	//let color = in.color * shading;
    let color = textureSample(gradientTexture, textureSampler, in.uv).rgb * shading;

    //let color = in.normal * 0.5 + 0.5;
	// Gamma-correction
	//let corrected_color = pow(color, vec3f(2.2));
	let corrected_color = color;
	return vec4f(corrected_color, u.color.a);
}