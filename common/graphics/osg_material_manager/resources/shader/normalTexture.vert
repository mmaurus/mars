void normalTexture(in vec3 n) {
  // get the tangent in world space (multiplication by gl_NormalMatrix
  // transforms to eye space)
  // the tangent should point in positive u direction on the uv plane in the tangent space.
  vec3 t = normalize( (osg_ViewMatrixInverse*vec4(gl_NormalMatrix * vertexTangent.xyz, 0.0)).xyz );
  // calculate the binormal, cross makes sure tbn matrix is orthogonal
  // multiplicated by handeness.
  vec3 b = cross(n, t);
  ttw = mat3(t, b, n);
}