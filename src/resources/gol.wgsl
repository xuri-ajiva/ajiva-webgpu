@group(0) @binding(0) var<storage,read> inputBuffer: array<f32>;
@group(0) @binding(1) var<storage,read_write> outputBuffer: array<f32>;
@group(0) @binding(2) var texture: texture_storage_2d<rgba8unorm,write>;

@compute @workgroup_size(1024, 1) fn compute(@builtin(global_invocation_id) id: vec3u) {
    /* game of live imple*/

    var x: u32 = id.x;

    for (var i: u32 = 0u; i < 1024u; i = i + 1u) {
        var index = x * 1024u + i;

        if(i == 0u || i == 31u || i == 992u || i == 1023u) {
            outputBuffer[index] = 0.0;
            continue;
        } else {
            var neighbors: f32 = 0.0
                    + inputBuffer[x * 1024u + i - 1u]
                    + inputBuffer[x * 1024u + i + 1u]
                    + inputBuffer[(x - 1u) * 1024u + i - 1u]
                    + inputBuffer[(x - 1u) * 1024u + i]
                    + inputBuffer[(x - 1u) * 1024u + i + 1u]
                    + inputBuffer[(x + 1u) * 1024u + i - 1u]
                    + inputBuffer[(x + 1u) * 1024u + i]
                    + inputBuffer[(x + 1u) * 1024u + i + 1u];

            if (inputBuffer[index] > 0.0) {
                if (neighbors < 1.9 || neighbors > 2.9) {
                    outputBuffer[index] = 0.0;
                } else {
                    outputBuffer[index] = 1.0;
                }
            } else {
                if (neighbors > 2.9 && neighbors < 3.1) {
                    outputBuffer[index] = 1.0;
                } else {
                    outputBuffer[index] = 0.0;
                }
            }

            let color = vec4<f32>(outputBuffer[index], outputBuffer[index], outputBuffer[index], 1.0);
            textureStore(texture, vec2(i, x), color);
        }
    }
}