#pragma once

// Basic reflection system based on "concepts" and template stuff
// 
// Objects that are reflected are supposed to contain a Reflector class variable
// 

#include <unordered_map>
#include <vector>
#include <string>
#include <type_traits>
#include "core/auxiliary/MetaTemplates.h"
#include "asset/AssetPod.h"


class StaticReflector;
class Reflector;

namespace impl
{
template<class T, class Sig, class = void>struct has_static_reflector :std::false_type {};

template<class T, class R, class...Args>
struct has_static_reflector <T, R(Args...),
	std::enable_if_t< // std:: in C++1y
	std::is_convertible<
	decltype(T::StaticReflect(std::declval<Args>()...)),
	R
	>::value
	&& !std::is_same<R, void>::value
	>
> : std::true_type {};

template<class T, class...Args>
struct has_static_reflector <T, void(Args...),
	decltype(void(T::foo(std::declval<Args>()...)))
> : std::true_type {};

template<typename T> struct has_reflector {
	struct Fallback { Reflector m_reflector; }; // introduce member name "x"
	struct Derived : T, Fallback { };

	template<typename C, C> struct ChT;

	template<typename C> static char(&f(ChT<Reflector Fallback::*, &C::m_reflector>*))[1];
	template<typename C> static char(&f(...))[2];

	static bool const value = sizeof(f<Derived>(0)) == 2;
};
}


template<typename T>
constexpr bool HasMemberReflector = impl::has_reflector<T>::value;

template<typename T>
constexpr bool HasStaticReflector = impl::has_static_reflector<T, const StaticReflector&()>::value;

template<typename T>
constexpr bool HasReflection = HasMemberReflector<T> || HasStaticReflector<T>;



template<typename T>
constexpr bool IsHandleToReflectedPodF()
{
	if constexpr (std::is_base_of_v<BasePodHandle, T>)
	{
		return HasStaticReflector<typename T::PodType>;
	}
	return false;
}

template<typename T>
constexpr bool IsHandleToReflectedPod = IsHandleToReflectedPodF<T>();


// All the currently supported reflected types.
enum class PropertyType
{
	NONE = 0,
	Int = 1,
	Bool,
	Float,
	Vec3,
	Vec4,
	String,
	
	Handle_Cubemap,
	Handle_GltfFile,
	Handle_Image,
	Handle_Material,
	Handle_Model,
	Handle_Shader,
	Handle_Text,
	Handle_Texture,
	Handle_XMLDoc,

	Vector_Cubemap,
	Vector_GltfFile,
	Vector_Image,
	Vector_Material,
	Vector_Model,
	Vector_Shader,
	Vector_Text,
	Vector_Texture,
	Vector_XMLDoc,
};

template<typename T>
constexpr bool IsReflectedF()
{
	if constexpr (IsHandleToReflectedPod<T>)
	{
		return true;
	}
	else if constexpr (is_vector_of_base_v<T, BasePodHandle>)
	{
		return true;
	}
	return false;
}


// IsReflected to compile time check if a type can be reflected.
template<typename Type> 
constexpr bool IsReflected = IsReflectedF<Type>();
template<> constexpr bool IsReflected<int32> = true;
template<> constexpr bool IsReflected<bool> = true;
template<> constexpr bool IsReflected<float> = true;
template<> constexpr bool IsReflected<glm::vec3> = true;
template<> constexpr bool IsReflected<glm::vec4> = true;
template<> constexpr bool IsReflected<std::string> = true;



struct GltfFilePod;
struct ImagePod;
struct MaterialPod;
struct ModelPod;
struct ShaderPod;
struct StringPod;
struct TexturePod;
struct XMLDocPod;

template<typename Type>
constexpr PropertyType ReflectionFromType = PropertyType::NONE;
template<> PropertyType ReflectionFromType<int32> = PropertyType::Int;
template<> PropertyType ReflectionFromType<bool> = PropertyType::Bool;
template<> PropertyType ReflectionFromType<float> = PropertyType::Float;
template<> PropertyType ReflectionFromType<glm::vec3> = PropertyType::Vec3;
template<> PropertyType ReflectionFromType<glm::vec4> = PropertyType::Vec4;
template<> PropertyType ReflectionFromType<std::string> = PropertyType::String;


template<> PropertyType ReflectionFromType<PodHandle<GltfFilePod>> = PropertyType::Handle_GltfFile;
template<> PropertyType ReflectionFromType<PodHandle<ImagePod>> = PropertyType::Handle_Image;
template<> PropertyType ReflectionFromType<PodHandle<MaterialPod>> = PropertyType::Handle_Material;
template<> PropertyType ReflectionFromType<PodHandle<ModelPod>> = PropertyType::Handle_Model;
template<> PropertyType ReflectionFromType<PodHandle<ShaderPod>> = PropertyType::Handle_Shader;
template<> PropertyType ReflectionFromType<PodHandle<StringPod>> = PropertyType::Handle_Text;
template<> PropertyType ReflectionFromType<PodHandle<TexturePod>> = PropertyType::Handle_Texture;
template<> PropertyType ReflectionFromType<PodHandle<XMLDocPod>> = PropertyType::Handle_XMLDoc;

template<> PropertyType ReflectionFromType<std::vector<PodHandle<GltfFilePod>>> = PropertyType::Vector_GltfFile;
template<> PropertyType ReflectionFromType<std::vector<PodHandle<ImagePod>>> = PropertyType::Vector_Image;
template<> PropertyType ReflectionFromType<std::vector<PodHandle<MaterialPod>>> = PropertyType::Vector_Material;
template<> PropertyType ReflectionFromType<std::vector<PodHandle<ModelPod>>> = PropertyType::Vector_Model;
template<> PropertyType ReflectionFromType<std::vector<PodHandle<ShaderPod>>> = PropertyType::Vector_Shader;
template<> PropertyType ReflectionFromType<std::vector<PodHandle<StringPod>>> = PropertyType::Vector_Text;
template<> PropertyType ReflectionFromType<std::vector<PodHandle<TexturePod>>> = PropertyType::Vector_Texture;
template<> PropertyType ReflectionFromType<std::vector<PodHandle<XMLDocPod>>> = PropertyType::Vector_XMLDoc;




// ALWAYS expected to run in member function
#define REFLECT_VAR(Variable, ...) GetReflector(this).AutoAddProperty(#Variable, Variable).InitFlags(PropertyFlags::Pack(__VA_ARGS__));


// Static reflection, this macro leaves the scope on public:
#define STATIC_REFLECTOR(Class)	private:			\
using __ThisType = Class; public:					\
Class() { type = ctti::type_id<Class>(); }			\
[[nodiscard]]										\
static const StaticReflector& StaticReflect()		\
{													\
	static StaticReflector reflector(#Class);		\
	if (!reflector.m_generated)						\
	{												\
		_FillMembers(reflector);					\
		reflector.m_generated = true;				\
	}												\
	return reflector;								\
} public:											\
static void _FillMembers(StaticReflector& reflector)\

// Static reflect var
#define S_REFLECT_VAR(Variable, ...)				\
	reflector.AutoAddProperty<decltype(Variable)>(#Variable, offsetof(__ThisType, Variable)).InitFlags(PropertyFlags::Pack(__VA_ARGS__));

//
// Example expansion
// 
// struct Class
// {
// 	using __ThisType = Class; 
// public:
// 	[[nodiscard]]
// 	static const StaticReflector& StaticReflect()
// 	{
// 		static StaticReflector reflector("Class");
// 		if (!reflector.m_generated)
// 		{
// 			_FillMembers(reflector);
// 			reflector.m_generated = true;
// 		}
// 		return reflector;
// 	} 
// public:
// 	static void _FillMembers(StaticReflector& reflector)
// 
// 	{
// 		reflector.AutoAddProperty<decltype(member[2])>("member", offsetof(__ThisType, member[2])).InitFlags(PropertyFlags::Pack(__VA_ARGS__));
// 	}
// 
// 	int member[10];
// };

/*
#define REFLECTED_POD(Class) private:				\
using __ThisType = Class; public:					\
[[nodiscard]]										\
static const StaticReflector& StaticReflect()		\
{													\
	static StaticReflector reflector(#Class);		\
	if (!reflector.m_generated)						\
	{												\
		_FillMembers(reflector);					\
		reflector.m_generated = true;				\
	}												\
	return reflector;								\
}													\
PodReflector GetPodReflector() override				\
{													\
	return std::move(__ThisType::StaticReflect())	\
			.ToAbsoluteReflector(object);			\
} public:											\
static void _FillMembers(StaticReflector& reflector)\
*/