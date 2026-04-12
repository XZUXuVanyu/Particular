#pragma once
//==============================================================================
/*
	A simple quaternion lib
*/
//==============================================================================
#include <iostream>
#include <format>
#include <JuceHeader.h>
#include <glm-master/glm/glm.hpp>
//==============================================================================
class Quaternion
{
public:
	//==============================================================================
	Quaternion();
	Quaternion(GLfloat real, GLfloat x, GLfloat y, GLfloat z);
	Quaternion(GLfloat scalar, const glm::vec3& vec);
	~Quaternion();

	Quaternion& operator+=(const Quaternion& q);
	Quaternion operator+(const Quaternion& q) const;
	Quaternion& operator-=(const Quaternion& q);
	Quaternion operator-(const Quaternion& q) const;
	
	Quaternion& operator*=(const Quaternion& other);
	Quaternion operator*(const Quaternion& other) const;
	friend std::ostream& operator<<(std::ostream& os, const Quaternion& q)
	{
		os << std::format("Quat[{:.2f},({:.2f},{:.2f},{:.2f})]", q.s, q.v.x, q.v.y, q.v.z);
		return os;
	}
	
	static GLdouble modulo(const Quaternion& q);
	Quaternion get_normalize() const;
	static void normalize(Quaternion& q);
	Quaternion get_conjugate() const;
	static void conjugate(Quaternion& q);
	Quaternion get_inverse() const;
	static void inverse(Quaternion& q);

	static Quaternion gen_rotater(GLfloat angle, const glm::vec3& axis);
	Quaternion get_rotate(GLfloat angle, const glm::vec3& axis) const;
	static void rotate(Quaternion& q_to_rotate, GLfloat angle, const glm::vec3& axis);
	static void rotate(glm::vec3& vec_to_rotate, GLfloat angle, const glm::vec3& axis);

	GLfloat s;
	glm::vec3 v;
	
};
//==============================================================================
inline Quaternion::Quaternion() :s(0), v(glm::vec3{ 0 }) {}
inline Quaternion::Quaternion(GLfloat real, GLfloat x, GLfloat y, GLfloat z)
	: s(real), v(glm::vec3(x, y, z))
{
};
inline Quaternion::Quaternion(GLfloat scalar, const glm::vec3& vec)
	: s(scalar), v(vec)
{
}
inline Quaternion::~Quaternion()
{
}
inline Quaternion&	Quaternion::operator+=(const Quaternion& q)
{
	this->s += q.s;
	this->v += q.v;
	return *this;
}
inline Quaternion	Quaternion::operator+(const Quaternion& q) const
{
	return Quaternion(this->s + q.s, this->v + q.v);
}
inline Quaternion&	Quaternion::operator-=(const Quaternion& q)
{
	this->s -= q.s;
	this->v -= q.v;
	return *this;
}
inline Quaternion	Quaternion::operator-(const Quaternion& q) const
{
	return Quaternion(this->s - q.s, this->v - q.v);
}
inline Quaternion& Quaternion::operator*=(const Quaternion& other)
{
	GLfloat new_s{ this->s * other.s - glm::dot(this->v, other.v) };
	glm::vec3 new_v{ this->s * other.v + this->v * other.s + glm::cross(this->v,other.v) };
	this->s = new_s, this->v = new_v;
	return *this;
}
inline Quaternion Quaternion::operator*(const Quaternion& other) const
{
	Quaternion result{ *this };
	result *= other;
	return result;
}

inline GLdouble Quaternion::modulo(const Quaternion& q)
{
	GLdouble modulo{ 0 };
	GLdouble squared_modulo{ q.s * q.s + glm::dot(q.v, q.v) };
	if (squared_modulo > 1e-14) modulo = sqrt(squared_modulo);
	return modulo;
}
inline Quaternion Quaternion::get_normalize() const
{
	Quaternion q{ *this };
	GLdouble m{ Quaternion::modulo(*this) };
	if (m == 0) q.s = 1.0f, q.v = glm::vec3(0.0f);
	else
	{
		GLdouble inversed_modulo{ 1.0 / m };
		q.s *= inversed_modulo, q.v *= inversed_modulo;
	}
	return q;
}
inline void Quaternion::normalize(Quaternion& q)
{
	GLdouble m{ Quaternion::modulo(q) };
	if (m == 0) q.s = 1.0f, q.v = glm::vec3(0.0f);
	else
	{
		GLdouble inversed_modulo{ 1.0 / m };
		q.s *= inversed_modulo, q.v *= inversed_modulo;
	}
}
inline Quaternion Quaternion::get_conjugate() const
{
	Quaternion q{ *this };
	q.v = -q.v;
	return q;
}
inline void Quaternion::conjugate(Quaternion& q)
{
	q.v = -q.v;
}
inline Quaternion Quaternion::get_inverse() const
{
	Quaternion q{ *this };
	GLdouble m{ Quaternion::modulo(q) };
	if (m == 0) q.s = 1.0f, q.v = glm::vec3(0.0f);
	else
	{
		GLdouble invsquared_modulo{ 1.0 / (m * m) };
		q.s *= (invsquared_modulo);
		q.v *= -(invsquared_modulo);
	}
	return q;
}
inline void Quaternion::inverse(Quaternion& q)
{
	GLdouble m{ Quaternion::modulo(q)};
	if (m == 0) q.s = 1.0f, q.v = glm::vec3(0.0f);
	else
	{
		GLdouble invsquared_modulo{ 1.0 / (m * m) };
		q.s *= (invsquared_modulo);
		q.v *= -(invsquared_modulo);
	}
}
inline Quaternion Quaternion::gen_rotater(GLfloat angle, const glm::vec3& axis)
{
	return Quaternion{ cos(angle / 2.0f), sin(angle / 2.0f) * glm::normalize(axis)};
}
inline Quaternion Quaternion::get_rotate(GLfloat angle, const glm::vec3& axis) const
{
	Quaternion rotater{ Quaternion::gen_rotater(angle,axis) };
	return rotater * (*this) * rotater.get_conjugate();
}
inline void Quaternion::rotate(Quaternion& q_to_rotate, GLfloat angle, const glm::vec3& axis)
{
	Quaternion rotater{ Quaternion::gen_rotater(angle,axis) };
	q_to_rotate = rotater * q_to_rotate * rotater.get_conjugate();
}
inline void Quaternion::rotate(glm::vec3& vec_to_rotate, GLfloat angle, const glm::vec3& axis)
{
	Quaternion rotater{ Quaternion::gen_rotater(angle,axis) };
	Quaternion q_to_rotate{ 0.0f, vec_to_rotate };

	q_to_rotate = rotater * q_to_rotate * rotater.get_conjugate();
	vec_to_rotate = q_to_rotate.v;
}
//==============================================================================