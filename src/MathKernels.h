#pragma once

#include <cmath>
#include "Common.h"
#include <Eigen/Geometry>
#include <iostream>

namespace MK {
template<typename value_type>
int is_almost_equal(value_type a, value_type b) {
	return std::abs(a - b) < 1e-6 ? 1 : 0;
}

template<typename value_type>
static inline Matrix<value_type> hatmap(const Eigen::Ref<const Vector<value_type>> &a) {
	Matrix<value_type> A(3, 3);
	A(0, 0) = 0;
	A(1, 0) = a(2);
	A(2, 0) = -a(1);
	A(0, 1) = -a(2);
	A(1, 1) = 0;
	A(2, 1) = a(0);
	A(0, 2) = a(1);
	A(1, 2) = -a(0);
	A(2, 2) = 0;

	return A;
}

template<typename value_type>
static inline Vector<value_type> hatmap_inv(const Eigen::Ref<const Matrix<value_type>> &A) {
	Vector<value_type> a(3);
	a(0) = A(2, 1);
	a(1) = A(0, 2);
	a(2) = A(1, 0);
	
	assert(is_almost_equal(A(2, 1), -A(1, 2)));
	assert(is_almost_equal(A(0, 2), -A(2, 0)));
	assert(is_almost_equal(A(1, 0), -A(0, 1)));

	return a;
}

template<typename value_type>
static inline Matrix<value_type> cay(const Eigen::Ref<const Vector<value_type>> &a) {
	value_type gamma = 1.0/(1.0 + a.squaredNorm()/4.0);
	
	Matrix<value_type> a_hat = hatmap<value_type>(a);

	Matrix<value_type> I = Matrix<value_type>::Identity(3, 3);
	Matrix<value_type> A(3, 3);
	A = I + gamma*(I + 0.5*a_hat)*a_hat;
	
	return A;
}

template<typename value_type>
static inline Matrix<value_type> cay_inv(const Eigen::Ref<const Matrix<value_type>> &A) {
	Matrix<value_type> B = 2.0*(A - A.transpose())/(1.0 + A.trace());
	
	return B;
}
template<typename value_type>
static inline void rodrigues_rotation(const Eigen::Ref<const Vector<value_type>> &a, const Eigen::Ref<const Vector<value_type>> &b, Eigen::Ref<Matrix<value_type>> A) {

	Vector<value_type> v = Vector3Ref<value_type>(a).cross(Vector3Ref<value_type>(b));

	value_type c = a.dot(b);

	Matrix<value_type> V0 = hatmap<value_type>(v, V0);

	value_type h0 = (value_type)(1.0f/(1.0f + c));
	A = V0 + h0*V0*V0;

	A(0, 0) += (value_type)1.0;
	A(1, 1) += (value_type)1.0;
	A(2, 2) += (value_type)1.0;
}

}
