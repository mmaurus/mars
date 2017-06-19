/**
 * this method returns a per fragment normal from a normal map.
 * the normal exists in tangent space.
 **/

void normalTexture(in vec4 texel, inout vec3 n) {
  n = n + (normalize(ttw*(texel.xyz * 2.0 - 1.0)) - n)*bumpNorFac;
  n = normalize(n);
}