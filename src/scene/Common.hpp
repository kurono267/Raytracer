//
// Created by kurono267 on 2018-12-26.
//

#pragma once

#include <algorithm>

inline float clamp(float v, float _min, float _max) {
	return std::max(std::min(v, _max), _min);
}

template<typename Predicate>
int findInterval(int size, const Predicate &pred) {
	int first = 0, len = size;
	while (len > 0) {
		int half = len >> 1, middle = first + half;
		if (pred(middle)) {
			first = middle + 1;
			len -= half + 1;
		} else
			len = half;
	}
	return clamp(first - 1, 0, size - 2);
}
