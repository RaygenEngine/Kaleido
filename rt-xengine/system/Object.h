#pragma once

class Object
{
	Core::UID m_id;

public:
	Object()
		: m_id (Core::UUIDGenerator::GenerateUUID()) {}
	virtual ~Object() = default;
	
	[[nodiscard]] Core::UID GetUID() const { return m_id; }

	// TODO: return ostream
	virtual void ToString(std::ostream& os) const { os << "ObjectId: " << m_id; }
};

class Engine;

template<typename T>
Engine* GetEngine(T objectContext) {
	static_assert("Cannot get engine from this object context.");
	return nullptr;
}