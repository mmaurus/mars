void defaultPhong() {
    vec4 vViewPos = gl_ModelViewMatrix * gl_Vertex;
    vec4 vWorldPos = osg_ViewMatrixInverse * vViewPos;
    for (int i=0; i<numLights; ++i) {
        if (lightIsSet[i] == 1) {
            if (lightIsDirectional[i] == 1) {
                lightDir[i] = -lightPos[i];
            } else {
                lightDir[i] = vWorldPos.xyz-lightPos[i];
            }
        }
    }
    eyeVec = osg_ViewMatrixInverse[3].xyz-vWorldPos.xyz;
}