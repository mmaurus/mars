void diffuseTexture(in vec2 texCoord, inout vec4 diffuse_base) {
    diffuse_base = texture2D(diffuseMap, texCoord);
}