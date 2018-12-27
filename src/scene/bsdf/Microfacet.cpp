//
// Created by kurono267 on 2018-12-27.
//

#include "Microfacet.hpp"

static void beckmannSample11(float cosThetaI, float U1, float U2,
							 float *slope_x, float *slope_y) {
	/* Special case (normal incidence) */
	if (cosThetaI > .9999) {
		float r = std::sqrt(-std::log(1.0f - U1));
		float sinPhi = std::sin(2 * (float)M_PI * U2);
		float cosPhi = std::cos(2 * (float)M_PI * U2);
		*slope_x = r * cosPhi;
		*slope_y = r * sinPhi;
		return;
	}

	/* The original inversion routine from the paper contained
	   discontinuities, which causes issues for QMC integration
	   and techniques like Kelemen-style MLT. The following code
	   performs a numerical inversion with better behavior */
	float sinThetaI =
			std::sqrt(std::max((float)0, (float)1 - cosThetaI * cosThetaI));
	float tanThetaI = sinThetaI / cosThetaI;
	float cotThetaI = 1 / tanThetaI;

	/* Search interval -- everything is parameterized
	   in the Erf() domain */
	float a = -1, c = erf(cotThetaI);
	float sample_x = std::max(U1, (float)1e-6f);

	/* Start with a good initial guess */
	// float b = (1-sample_x) * a + sample_x * c;

	/* We can do better (inverse of an approximation computed in
	 * Mathematica) */
	float thetaI = std::acos(cosThetaI);
	float fit = 1 + thetaI * (-0.876f + thetaI * (0.4265f - 0.0594f * thetaI));
	float b = c - (1 + c) * std::pow(1 - sample_x, fit);

	/* Normalization factor for the CDF */
	static const float SQRT_PI_INV = 1.f / std::sqrt((float)M_PI);
	float normalization =
			1 /
			(1 + c + SQRT_PI_INV * tanThetaI * std::exp(-cotThetaI * cotThetaI));

	int it = 0;
	while (++it < 10) {
		/* Bisection criterion -- the oddly-looking
		   Boolean expression are intentional to check
		   for NaNs at little additional cost */
		if (!(b >= a && b <= c)) b = 0.5f * (a + c);

		/* Evaluate the CDF and its derivative
		   (i.e. the density function) */
		float invErf = erfc(b);
		float value =
				normalization *
				(1 + b + SQRT_PI_INV * tanThetaI * std::exp(-invErf * invErf)) -
				sample_x;
		float derivative = normalization * (1 - invErf * tanThetaI);

		if (std::abs(value) < 1e-5f) break;

		/* Update bisection intervals */
		if (value > 0)
			c = b;
		else
			a = b;

		b -= value / derivative;
	}

	/* Now convert back into a slope value */
	*slope_x = erfc(b);

	/* Simulate Y component */
	*slope_y = erfc(2.0f * std::max(U2, (float)1e-6f) - 1.0f);
}

static glm::vec3 beckmannSample(const glm::vec3 &wi, float alpha_x, float alpha_y,
							   float U1, float U2) {
	// 1. stretch wi
	glm::vec3 wiStretched =
			glm::normalize(glm::vec3(alpha_x * wi.x, alpha_y * wi.y, wi.z));

	// 2. simulate P22_{wi}(x_slope, y_slope, 1, 1)
	float slope_x, slope_y;
	beckmannSample11(cosTheta(wiStretched), U1, U2, &slope_x, &slope_y);

	// 3. rotate
	float tmp = cosPhi(wiStretched) * slope_x - sinPhi(wiStretched) * slope_y;
	slope_y = sinPhi(wiStretched) * slope_x + cosPhi(wiStretched) * slope_y;
	slope_x = tmp;

	// 4. unstretch
	slope_x = alpha_x * slope_x;
	slope_y = alpha_y * slope_y;

	// 5. compute normal
	return glm::normalize(glm::vec3(-slope_x, -slope_y, 1.f));
}

// MicrofacetDistribution Method Definitions
MicrofacetDistribution::~MicrofacetDistribution() {}

float BeckmannDistribution::D(const glm::vec3 &wh) const {
	float _tan2Theta = tan2Theta(wh);
	if (std::isinf(_tan2Theta)) return 0.f;
	float cos4Theta = cos2Theta(wh) * cos2Theta(wh);
	return std::exp(-_tan2Theta * (cos2Phi(wh) / (alphax * alphax) +
								  sin2Phi(wh) / (alphay * alphay))) /
		   ((float)M_PI * alphax * alphay * cos4Theta);
}

float BeckmannDistribution::lambda(const glm::vec3 &w) const {
	float absTanTheta = std::abs(tanTheta(w));
	if (std::isinf(absTanTheta)) return 0.f;
	// Compute _alpha_ for direction _w_
	float alpha =
			std::sqrt(cos2Phi(w) * alphax * alphax + sin2Phi(w) * alphay * alphay);
	float a = 1 / (alpha * absTanTheta);
	if (a >= 1.6f) return 0;
	return (1 - 1.259f * a + 0.396f * a * a) / (3.535f * a + 2.181f * a * a);
}

glm::vec3 BeckmannDistribution::sampleWh(const glm::vec3 &wo,
										 const glm::vec2 &u) const {
	if (!sampleVisibleArea) {
		// Sample full distribution of normals for Beckmann distribution

		// Compute $\tan^2 \theta$ and $\phi$ for Beckmann distribution sample
		float tan2Theta, phi;
		if (alphax == alphay) {
			float logSample = std::log(1 - u[0]);
			tan2Theta = -alphax * alphax * logSample;
			phi = u[1] * 2 * (float)M_PI;
		} else {
			// Compute _tan2Theta_ and _phi_ for anisotropic Beckmann
			// distribution
			float logSample = std::log(1 - u[0]);
			phi = std::atan(alphay / alphax *
							std::tan(2 * (float)M_PI * u[1] + 0.5f * (float)M_PI));
			if (u[1] > 0.5f) phi += (float)M_PI;
			float sinPhi = std::sin(phi), cosPhi = std::cos(phi);
			float alphax2 = alphax * alphax, alphay2 = alphay * alphay;
			tan2Theta = -logSample /
						(cosPhi * cosPhi / alphax2 + sinPhi * sinPhi / alphay2);
		}

		// Map sampled Beckmann angles to normal direction _wh_
		float cosTheta = 1 / std::sqrt(1 + tan2Theta);
		float sinTheta = std::sqrt(std::max((float)0, 1 - cosTheta * cosTheta));
		glm::vec3 wh = sphericalDirection(sinTheta, cosTheta, phi);
		if (!sameHemisphere(wo, wh)) wh = -wh;
		return wh;
	} else {
		// Sample visible area of normals for Beckmann distribution
		glm::vec3 wh;
		bool flip = wo.z < 0;
		wh = beckmannSample(flip ? -wo : wo, alphax, alphay, u[0], u[1]);
		if (flip) wh = -wh;
		return wh;
	}
}

float MicrofacetDistribution::pdf(const glm::vec3 &wo,
								  const glm::vec3 &wh) const {
	if (sampleVisibleArea)
		return D(wh) * G1(wo) * std::abs(glm::dot(wo, wh)) / absCosTheta(wo);
	else
		return D(wh) * absCosTheta(wh);
}

glm::vec3 FresnelConductor::operator()(float cosI) const {
	auto cosThetaI = clamp(cosI, -1, 1);
	auto eta = etaT / etaI;
	auto etak = k / etaI;

	float cosThetaI2 = cosThetaI * cosThetaI;
	float sinThetaI2 = 1.f - cosThetaI2;
	glm::vec3 eta2 = eta * eta;
	glm::vec3 etak2 = etak * etak;

	glm::vec3 t0 = eta2 - etak2 - sinThetaI2;
	glm::vec3 a2plusb2 = glm::sqrt(t0 * t0 + 4.f * eta2 * etak2);
	glm::vec3 t1 = a2plusb2 + cosThetaI2;
	glm::vec3 a = glm::sqrt(0.5f * (a2plusb2 + t0));
	glm::vec3 t2 = 2.f * cosThetaI * a;
	glm::vec3 Rs = (t1 - t2) / (t1 + t2);

	glm::vec3 t3 = cosThetaI2 * a2plusb2 + sinThetaI2 * sinThetaI2;
	glm::vec3 t4 = t2 * sinThetaI2;
	glm::vec3 Rp = Rs * (t3 - t4) / (t3 + t4);

	return 0.5f * (Rp + Rs);
}

glm::vec3 Reflection::f(const glm::vec3 &wo, const glm::vec3 &wi) const {
	float cosThetaO = absCosTheta(wo), cosThetaI = absCosTheta(wi);
	glm::vec3 wh = wi + wo;
	// Handle degenerate cases for microfacet reflection
	if (cosThetaI == 0 || cosThetaO == 0) return glm::vec3(0.f);
	if (wh.x == 0 && wh.y == 0 && wh.z == 0) return glm::vec3(0.f);
	wh = glm::normalize(wh);
	glm::vec3 F = (*fresnel)(glm::dot(wi, wh));
	return R * distribution->D(wh) * distribution->G(wo, wi) * F /
		   (4 * cosThetaI * cosThetaO);
}

glm::vec3 Reflection::sampled(const glm::vec3& out, const glm::vec2& sample, glm::vec3& in, float& pdf, Type& type) const {
	// Sample microfacet orientation $\wh$ and reflected direction $\wi$
	if (out.z == 0) return glm::vec3(0.f);
	auto wh = distribution->sampleWh(out, sample);
	in = glm::reflect(out, wh);
	if (!sameHemisphere(out, in)) return glm::vec3(0.f);

	// Compute PDF of _wi_ for microfacet reflection
	pdf = distribution->pdf(out, wh) / (4.f * glm::dot(out, wh));
	return f(out, in);
}

float Reflection::pdf(const glm::vec3 &wo, const glm::vec3 &wi) const {
	if (!sameHemisphere(wo, wi)) return 0;
	auto wh = glm::normalize(wo + wi);
	return distribution->pdf(wo, wh) / (4.f * glm::dot(wo, wh));
}
