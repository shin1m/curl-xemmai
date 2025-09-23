#ifndef XEMMAIX__CURL__IO_H
#define XEMMAIX__CURL__IO_H

#include "curl.h"

namespace xemmaix::curl
{

struct t_request : t_proxy
{
	CURL* v_curl = curl_easy_init();
	curl_slist* v_headers = NULL;
	t_root v_buffer = t_bytes::f_instantiate(CURL_MAX_WRITE_SIZE);

	t_request(std::wstring_view a_url);
	virtual void f_dispose();
	template<typename T>
	void f_option(CURLoption a_option, T&& a_value)
	{
		auto code = curl_easy_setopt(v_curl, a_option, std::forward<T>(a_value));
		if (code != CURLE_OK) throw std::runtime_error("curl_easy_setopt("s + curl_easy_option_by_id(a_option)->name + "): " + curl_easy_strerror(code));
	}
	void f_send()
	{
		auto code = curl_multi_add_handle(v_session->v_curlm, v_curl);
		if (code != CURLM_OK) throw std::runtime_error("curl_multi_add_handle: "s + curl_multi_strerror(code));
	}
};

}

namespace xemmai
{

template<>
struct t_type_of<xemmaix::curl::t_request> : t_derivable<t_bears<xemmaix::curl::t_request, t_type_of<xemmaix::curl::t_proxy>>>
{
	static void f_define(t_library* a_library);

	using t_base::t_base;
	t_pvalue f_do_construct(t_pvalue* a_stack, size_t a_n);
};

}

#endif
