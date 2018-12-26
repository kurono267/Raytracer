//
// Created by kurono267 on 2018-12-26.
//

#include "Distributions.hpp"

Distribution1D::Distribution1D(const std::vector<float> &data) : func(data) {
	cdf.resize(data.size() + 1);
	cdf[0] = 0.f;
	auto n = data.size();
	for (int i = 1; i < cdf.size(); ++i) {
		cdf[i] = (cdf[i - 1] + func[i - 1]) / (float) n;
	}
	funcInt = cdf[n];
	if (funcInt == 0) {
		for (int i = 1; i < cdf.size(); ++i)
			cdf[i] = (float) i / (float) n;
	} else {
		for (int i = 1; i < cdf.size(); ++i)
			cdf[i] /= funcInt;
	}
}

int Distribution1D::count() const { return func.size(); }

float Distribution1D::sampleContinuous(float u, float &pdf, int *off) const {
	int offset = findInterval(cdf.size(), [&](int index) { return cdf[index] <= u; });
	if (off) *off = offset;
	float du = u - cdf[offset];
	if ((cdf[offset + 1] - cdf[offset]) > 0)
		du /= (cdf[offset + 1] - cdf[offset]);

	pdf = func[offset] / funcInt;
	return (offset + du) / count();
}

int Distribution1D::sampleDiscrete(float u, float& pdf, float *uRemapped) const {
	int offset = findInterval(cdf.size(), [&](int index) { return cdf[index] <= u; });
	pdf = func[offset] / (funcInt * count());
	if (uRemapped) *uRemapped = (u - cdf[offset]) / (cdf[offset + 1] - cdf[offset]);
	return offset;
}

float Distribution1D::discretePDF(int index) const { return func[index] / (funcInt * count()); }

Distribution2D::Distribution2D(const std::vector<float>& data, int width, int height){
	for (int v = 0; v < height; ++v) {
		pConditionalV.emplace_back(new Distribution1D(std::vector<float>(data.begin()+v*width,data.begin()+(v+1)*width)));
	}
	std::vector<float> marginalFunc;
	for (int v = 0; v < height; ++v)
		marginalFunc.push_back(pConditionalV[v]->funcInt);
	pMarginal.reset(new Distribution1D(std::vector<float>(marginalFunc.begin(),marginalFunc.begin()+height)));
}

glm::vec2 Distribution2D::sampleContinuous(const glm::vec2 &u, float& pdf) const {
	float pdfs[2];
	int v;
	float d1 = pMarginal->sampleContinuous(u[1], pdfs[1], &v);
	float d0 = pConditionalV[v]->sampleContinuous(u[0], pdfs[0]);
	pdf = pdfs[0] * pdfs[1];
	return glm::vec2(d0, d1);
}

float Distribution2D::pdf(const glm::vec2 &p) const {
	int iu = clamp(int(p[0] * pConditionalV[0]->count()),
				   0, pConditionalV[0]->count() - 1);
	int iv = clamp(int(p[1] * pMarginal->count()),
				   0, pMarginal->count() - 1);
	return pConditionalV[iv]->func[iu] / pMarginal->funcInt;
}
