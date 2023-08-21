
struct UniformData {
    projectionMatrix: mat4x4f,
    viewMatrix: mat4x4f,
    worldPos: vec3f,
    time: f32,
};

struct Light {
    position: vec4f,
    color: vec4f
};
struct LightningUniform{
    lights: array<Light, 4>,
    ambient: vec4f,
    hardness: f32,
    kd: f32,
    ks: f32,
};
struct InstanceInput {
    @location(10) model_matrix_0: vec4<f32>,
    @location(11) model_matrix_1: vec4<f32>,
    @location(12) model_matrix_2: vec4<f32>,
    @location(13) model_matrix_3: vec4<f32>,
    @location(14) instanceColor: vec4f,
};

struct VertexInput {
	@location(0) position: vec3f,
	@location(1) normal: vec3f,
	@location(2) color: vec3f,
    @location(3) uv: vec2f,
};

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(1) worldPosition: vec3f,
    @location(2) viewDirection: vec3<f32>,
    @location(3) normal: vec3f,
    @location(4) color: vec3f,
    @location(5) uv: vec2f,
};

@group(0) @binding(0) var<uniform> u: UniformData;
@group(0) @binding(1) var textureSampler: sampler;
@group(0) @binding(2) var gradientTexture: texture_2d<f32>;
@group(0) @binding(3) var normalTexture: texture_2d<f32>;
@group(0) @binding(4) var<uniform> l: LightningUniform;

const pi = 3.14159265359;

@vertex
fn vs_main(in: VertexInput, instance: InstanceInput) -> VertexOutput {
    let model_matrix = mat4x4<f32>(
        instance.model_matrix_0,
        instance.model_matrix_1,
        instance.model_matrix_2,
        instance.model_matrix_3,
    );
    var out: VertexOutput;
    let worldPosition = model_matrix * vec4<f32>(in.position, 1.0);
    out.position = u.projectionMatrix * u.viewMatrix * worldPosition;
    out.worldPosition = worldPosition.xyz;
    out.viewDirection = u.worldPos - worldPosition.xyz;
    out.normal = (model_matrix * vec4f(in.normal, 0.0)).xyz;
    out.color = in.color;
    out.uv = in.uv;
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
	//let N = normalize(in.normal);
	let encodedN = textureSample(normalTexture, textureSampler, in.uv).rgb;
    let N = normalize(encodedN - 0.5);
	let V = normalize(in.viewDirection);
	var diffuse = in.color.rgb * l.ambient.rgb;
	var specular = vec3<f32>(0.0);

	//point lights
	for (var i = 0; i < 4; i++) {
        let light = l.lights[i];
        //compute light pos relative to model
        let L = normalize(light.position.xyz - in.worldPosition);
        //compute falloff based on intensity (pos w) and distance
        let falloff = light.color.a / length(light.position.xyz - in.worldPosition);

		let R = -reflect(L, N); // equivalent to 2.0 * dot(N, L) * N - L
		let RoV = max(0.0, dot(R, V));
		let LoN = max(0.0, dot(L, N));

		let color = light.color.rgb * falloff;

        diffuse += color * in.color * LoN;
        specular += color * pow(RoV, l.hardness);
    }

    let baseColor = textureSample(gradientTexture, textureSampler, in.uv).rgb;

	let color = baseColor * l.kd * diffuse + l.ks * specular;

    //let color = in.normal * 0.5 + 0.5;
	// Gamma-correction
	//let corrected_color = pow(color, vec3f(2.2));
	let corrected_color = color;
	return vec4f(corrected_color, 1.0f);
}