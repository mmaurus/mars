void defaultPhong(in vec3 n, in vec4 ambient_base, in vec4 diffuse_base, in vec4 specular_base, inout vec4 Ia, inout vec4 Id, inout vec4 Is) {
	vec3 reflected;
	float nDotL, rDotE, atten;
	float spot = 1.0;
	for(int i=0; i<numLights; ++i) {
	    if(lightIsDirectional[i] == 1) {
                atten = lightConstantAtt[i];
        } else {
            float dist = length(lightDir[i]);
            atten = 1.0/(lightConstantAtt[i] + lightLinearAtt[i] * dist + lightQuadraticAtt[i] * dist * dist);
        }
		if(lightIsSet[i]==1) {
			nDotL = max(dot(normal, normalize(-lightDir[i])), 0.0);
			reflected = normalize(reflect(lightDir[i], n));
			rDotE = max(dot(reflected, normalize(eyeVec)), 0.0);
		}
		if(lightIsSpot[i] == 1) {
			float spotEffect = dot(normalize(lightSpotDir[i]), normalize(lightDir[i]));
			spot = (spotEffect > lightCosCutoff[i]) ? 1.0 : 1.0-min(1.0, pow(lightSpotExponent[i]*(lightCosCutoff[i]-spotEffect), 2));
		}
		Ia += lightAmbient[i]*ambient_base;
		Id += spot*atten*lightDiffuse[i]*diffuse_base*nDotL;
		Is +=  gl_FrontMaterial.shininess > 0 ? spot*atten*lightSpecular[i]*specular_base*pow(rDotE, gl_FrontMaterial.shininess) : vec4(0.0);
	}
}