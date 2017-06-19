 void noise(inout vec4 If) {
    if(useNoise == 1) {
        vec4 screenPos = (gl_ModelViewProjectionMatrix * modelVertex);
        screenPos /= screenPos.w;
        float noiseScale = 6;
        If.rg += 0.05*(texture2D( NoiseMap, noiseScale*screenPos.xy).zw-0.5);
        If.b += 0.05*(texture2D( NoiseMap, noiseScale*(screenPos.xy+vec2(0.5, 0.5))).z-0.5);
   }
 }