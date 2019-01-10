//
// Created by kurono267 on 2018-12-27.
//

#pragma once

#include "BxDF.hpp"

class MicrofacetDistribution {
	public:
		virtual ~MicrofacetDistribution();
		virtual float D(const glm::vec3 &wh) const = 0;
		virtual float lambda(const glm::vec3 &w) const = 0;
		float G1(const glm::vec3 &w) const {
			//    if (Dot(w, wh) * CosTheta(w) < 0.) return 0.;
			return 1 / (1 + lambda(w));
		}
		virtual float G(const glm::vec3 &wo, const glm::vec3 &wi) const {
			return 1 / (1 + lambda(wo) + lambda(wi));
		}
		virtual glm::vec3 sampleWh(const glm::vec3 &wo, const glm::vec2 &u) const = 0;
		float pdf(const glm::vec3 &wo, const glm::vec3 &wh) const;
	protected:
		// MicrofacetDistribution Protected Methods
		MicrofacetDistribution(bool sampleVisibleArea)
				: sampleVisibleArea(sampleVisibleArea) {}

		// MicrofacetDistribution Protected Data
		const bool sampleVisibleArea;
};

class BeckmannDistribution : public MicrofacetDistribution {
public:
	static float roughnessToAlpha(float roughness) {
		roughness = std::max(roughness, (float)1e-3);
		float x = std::log(roughness);
		return 1.62142f + 0.819955f * x + 0.1734f * x * x +
			   0.0171201f * x * x * x + 0.000640711f * x * x * x * x;
	}
	BeckmannDistribution(float alphax, float alphay, bool samplevis = true)
			: MicrofacetDistribution(samplevis), alphax(alphax), alphay(alphay) {}
	float D(const glm::vec3 &wh) const;
	glm::vec3 sampleWh(const glm::vec3 &wo, const glm::vec2 &u) const final;
private:
	// BeckmannDistribution Private Methods
	float lambda(const glm::vec3& w) const final;

	// BeckmannDistribution Private Data
	const float alphax, alphay;
};

class Fresnel {
	public:
		// Fresnel Interface
		virtual ~Fresnel() = default;
		virtual glm::vec3 operator()(float cosI) const = 0;
};

class FresnelConductor : public Fresnel {
	public:
		FresnelConductor(const glm::vec3 &etaI, const glm::vec3 &etaT,
						 const glm::vec3 &k)
				: etaI(etaI), etaT(etaT), k(k) {}
		glm::vec3 operator()(float cosI) const override;
	private:
		glm::vec3 etaI, etaT, k;
};

// Microfacet BSDFs based at Torrance Sparrow
class Reflection : public BxDF {
public:
	Reflection(const glm::vec3 &_R,
						 std::shared_ptr<MicrofacetDistribution> _distribution, std::shared_ptr<Fresnel> _fresnel)
			: BxDF(BxDF::REFLECTION | BxDF::GLOSSY),
			  R(_R),
			  distribution(_distribution),
			  fresnel(_fresnel) {}
  	~Reflection() override = default;
	glm::vec3 f(const glm::vec3& out, const glm::vec3& in) const override;
	glm::vec3 sampled(const glm::vec3& out, const glm::vec2& sample, glm::vec3& in, float& pdf, Type& type) const override;
	float pdf(const glm::vec3 &wo, const glm::vec3 &wi) const override;
private:
	// MicrofacetReflection Private Data
	const glm::vec3 R;
	const std::shared_ptr<MicrofacetDistribution> distribution;
	const std::shared_ptr<Fresnel> fresnel;
};

