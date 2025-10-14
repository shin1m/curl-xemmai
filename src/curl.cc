#include "request.h"

namespace xemmaix::curl
{

t_session::~t_session()
{
	while (v_next != this) static_cast<t_proxy*>(v_next)->f_dispose();
	curl_multi_cleanup(v_curlm);
	v_instance = nullptr;
}

void t_proxy::f_dispose()
{
	v_object = nullptr;
	this->~t_entry();
}

t_library::~t_library()
{
	curl_global_cleanup();
}

void t_library::f_scan(t_scan a_scan)
{
	XEMMAIX__CURL__TYPES(XEMMAI__TYPE__SCAN)
}

std::vector<std::pair<t_root, t_rvalue>> t_library::f_define()
{
	curl_global_init(CURL_GLOBAL_DEFAULT);
	t_type_of<t_proxy>::f_define(this);
	t_type_of<t_request>::f_define(this);
	return t_define(this)
	(L"Proxy"sv, static_cast<t_object*>(v_type_proxy))
	(L"Request"sv, static_cast<t_object*>(v_type_request))
	(L"main"sv, t_static<void(*)(const t_pvalue&), [](auto a_callable)
	{
		t_session session;
		a_callable();
	}>())
	(L"perform"sv, t_static<int(*)(), []
	{
		auto curlm = t_session::f_instance()->v_curlm;
		int running;
		auto code = curl_multi_perform(curlm, &running);
		if (code != CURLM_OK) throw std::runtime_error("curl_multi_perform: "s + curl_multi_strerror(code));
		while (true) {
			int n;
			auto m = curl_multi_info_read(curlm, &n);
			if (!m) break;
			if (m->msg == CURLMSG_DONE) {
				void* p;
				auto code = curl_easy_getinfo(m->easy_handle, CURLINFO_PRIVATE, &p);
				if (code != CURLE_OK) throw std::runtime_error("curl_easy_getinfo: "s + curl_easy_strerror(code));
				if (auto& on = t_object::f_of(p)->f_fields()[0/*on_done*/]) try {
					on(m->data.result == CURLE_OK ? nullptr : t_string::f_instantiate(portable::f_convert(curl_easy_strerror(m->data.result))));
				} catch (...) {
					if (auto q = static_cast<t_request*>(p); q->f_valid()) q->f_dispose();
					throw;
				}
				if (auto q = static_cast<t_request*>(p); q->f_valid()) q->f_dispose();
			}
		}
		return running;
	}>())
	(L"timeout"sv, t_static<long(*)(), []
	{
		auto curlm = t_session::f_instance()->v_curlm;
		long timeout;
		auto code = curl_multi_timeout(curlm, &timeout);
		if (code != CURLM_OK) throw std::runtime_error("curl_multi_timeout: "s + curl_multi_strerror(code));
		return timeout;
	}>())
	(L"waitfds"sv, t_static<t_object*(*)(), []
	{
		auto curlm = t_session::f_instance()->v_curlm;
		unsigned int n;
		auto code = curl_multi_waitfds(curlm, NULL, 0, &n);
		if (code != CURLM_OK) throw std::runtime_error("curl_multi_waitfds: "s + curl_multi_strerror(code));
		std::vector<curl_waitfd> fds(n);
		code = curl_multi_waitfds(curlm, fds.data(), n, &n);
		if (code != CURLM_OK) throw std::runtime_error("curl_multi_waitfds: "s + curl_multi_strerror(code));
		return t_tuple::f_instantiate(n, [&](auto& tuple)
		{
			for (size_t i = 0; i < n; ++i) {
				auto& fd = fds[i];
				new(&tuple[i]) t_svalue(f_tuple(fd.fd, fd.events));
			}
		});
	}>())
	(L"POLLIN"sv, CURL_WAIT_POLLIN)
	(L"POLLPRI"sv, CURL_WAIT_POLLPRI)
	(L"POLLOUT"sv, CURL_WAIT_POLLOUT)
	;
}

}

namespace xemmai
{

void t_type_of<xemmaix::curl::t_proxy>::f_define(t_library* a_library)
{
	using xemmaix::curl::t_proxy;
	t_define{a_library}
	(L"dispose"sv, t_member<void(t_proxy::*)(), &t_proxy::f_dispose>())
	.f_derive<t_proxy, t_object>();
}

}

XEMMAI__MODULE__FACTORY(xemmai::t_library::t_handle* a_handle)
{
	return xemmai::f_new<xemmaix::curl::t_library>(a_handle);
}
