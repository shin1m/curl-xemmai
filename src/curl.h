#ifndef XEMMAIX__CURL__CURL_H
#define XEMMAIX__CURL__CURL_H

#include <curl/curl.h>
#include <xemmai/convert.h>

namespace xemmaix::curl
{

using namespace xemmai;

class t_entry
{
protected:
	t_entry* v_previous;
	t_entry* v_next;

	t_entry() : v_previous(this), v_next(this)
	{
	}
	t_entry(t_entry* a_previous) : v_previous(a_previous), v_next(a_previous->v_next)
	{
		v_previous->v_next = v_next->v_previous = this;
	}

public:
	virtual void f_dispose();
};

class t_session : public t_entry
{
	static inline XEMMAI__PORTABLE__THREAD t_session* v_instance;

public:
	static t_session* f_instance()
	{
		if (!v_instance) f_throw(L"must be inside main."sv);
		return v_instance;
	}

	CURLM* v_curlm = curl_multi_init();

	t_session()
	{
		if (v_instance) f_throw(L"already inside main."sv);
		v_instance = this;
	}
	~t_session()
	{
		while (v_next != this) v_next->f_dispose();
		curl_multi_cleanup(v_curlm);
		v_instance = nullptr;
	}
};

class t_proxy : t_entry
{
protected:
	t_session* v_session = static_cast<t_session*>(v_previous);
	t_root v_object = t_object::f_of(this);

	t_proxy() : t_entry(t_session::f_instance())
	{
	}

public:
	virtual void f_dispose();
	bool f_valid()
	{
		return v_session == t_session::f_instance() && v_object;
	}
};

struct t_request;

class t_library : public xemmai::t_library
{
#define XEMMAIX__CURL__TYPES(_)\
	_(proxy)\
	_(request)
	XEMMAIX__CURL__TYPES(XEMMAI__TYPE__DECLARE)

public:
	using xemmai::t_library::t_library;
	virtual ~t_library();
	XEMMAI__LIBRARY__MEMBERS
};

XEMMAI__LIBRARY__BASE(t_library, t_global, f_global())
#define XEMMAI__TYPE__LIBRARY t_library
XEMMAIX__CURL__TYPES(XEMMAI__TYPE__DEFINE)
#undef XEMMAI__TYPE__LIBRARY

}

namespace xemmai
{

template<>
struct t_type_of<xemmaix::curl::t_proxy> : t_bears<xemmaix::curl::t_proxy>
{
	template<typename T>
	static T& f_cast(auto&& a_object)
	{
		auto& p = static_cast<t_object*>(a_object)->f_as<T>();
		if (!p.f_valid()) f_throw(L"not valid."sv);
		return p;
	}
	template<typename T>
	struct t_cast
	{
		static T f_as(auto&& a_object)
		{
			return std::forward<T>(f_cast<typename t_fundamental<T>::t_type>(std::forward<decltype(a_object)>(a_object)));
		}
		static bool f_is(t_object* a_object)
		{
			return reinterpret_cast<uintptr_t>(a_object) >= c_tag__OBJECT && a_object->f_type()->f_derives<typename t_fundamental<T>::t_type>();
		}
	};
	template<typename T>
	struct t_cast<T*>
	{
		static T* f_as(auto&& a_object)
		{
			return static_cast<t_object*>(a_object) ? &f_cast<typename t_fundamental<T>::t_type>(std::forward<decltype(a_object)>(a_object)) : nullptr;
		}
		static bool f_is(t_object* a_object)
		{
			return reinterpret_cast<uintptr_t>(a_object) == c_tag__NULL || reinterpret_cast<uintptr_t>(a_object) >= c_tag__OBJECT && a_object->f_type()->f_derives<typename t_fundamental<T>::t_type>();
		}
	};

	using t_library = xemmaix::curl::t_library;

	static void f_define(t_library* a_library);

	using t_base::t_base;
};

}

#endif
