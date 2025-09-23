#include "request.h"
#include <xemmai/engine.h>

namespace xemmaix::curl
{

t_request::t_request(std::wstring_view a_url)
{
	curl_easy_setopt(v_curl, CURLOPT_PRIVATE, this);
	curl_easy_setopt(v_curl, CURLOPT_READFUNCTION, +[](char* a_p, size_t a_size, size_t a_n, void* a_user) -> size_t
	{
		auto self = static_cast<t_request*>(a_user);
		if (auto& on = t_object::f_of(self)->f_fields()[1/*on_read*/]) try {
			auto& bytes = self->v_buffer->f_as<t_bytes>();
			a_n *= a_size;
			if (a_n > bytes.f_size()) a_n = bytes.f_size();
			auto size = on(self->v_buffer, a_n);
			f_check<size_t>(size, L"result of on_read");
			auto n = f_as<size_t>(size);
			if (n > a_n) f_throw(L"too large"sv);
			std::copy_n(&bytes[0], n, a_p);
			return n;
		} catch (...) {
			return CURL_READFUNC_ABORT;
		}
		return 0;
	});
	curl_easy_setopt(v_curl, CURLOPT_READDATA, this);
	curl_easy_setopt(v_curl, CURLOPT_WRITEFUNCTION, +[](char* a_p, size_t a_size, size_t a_n, void* a_user) -> size_t
	{
		auto self = static_cast<t_request*>(a_user);
		a_n *= a_size;
		if (auto& on = t_object::f_of(self)->f_fields()[2/*on_write*/]) try {
			std::copy_n(a_p, a_n, &self->v_buffer->f_as<t_bytes>()[0]);
			on(self->v_buffer, a_n);
		} catch (...) {
			return CURL_WRITEFUNC_ERROR;
		}
		return a_n;
	});
	curl_easy_setopt(v_curl, CURLOPT_WRITEDATA, this);
	curl_easy_setopt(v_curl, CURLOPT_URL, portable::f_convert(a_url).c_str());
}

void t_request::f_dispose()
{
	auto code = curl_multi_remove_handle(v_session->v_curlm, v_curl);
	if (code != CURLM_OK) throw std::runtime_error("curl_multi_remove_handle: "s + curl_multi_strerror(code));
	curl_easy_cleanup(v_curl);
	curl_slist_free_all(v_headers);
	v_buffer = nullptr;
	t_proxy::f_dispose();
}

}

namespace xemmai
{

void t_type_of<xemmaix::curl::t_request>::f_define(t_library* a_library)
{
	using namespace xemmaix::curl;
	t_define{a_library}
	(L"on_done"sv)
	(L"on_read"sv)
	(L"on_write"sv)
	(L"follow_location"sv, t_member<void(*)(t_request&, long), [](auto a_self, auto a_mode)
	{
		a_self.f_option(CURLOPT_FOLLOWLOCATION, a_mode);
	}>())
	(L"http_headers"sv, t_member<void(*)(t_request&, const t_pvalue&), [](auto a_self, auto a_headers)
	{
		auto& headers = a_self.v_headers;
		curl_slist_free_all(headers);
		a_self.f_option(CURLOPT_HTTPHEADER, headers = NULL);
		if (!a_headers) return;
		auto append = [&](size_t n, auto get)
		{
			for (size_t i = 0; i < n; ++i) {
				auto s = get(i);
				f_check<std::wstring_view>(s, L"header");
				auto list = curl_slist_append(headers, portable::f_convert(f_as<std::wstring_view>(s)).c_str());
				if (!list) f_throw(L"curl_slist_append"sv);
				headers = list;
			}
		};
		auto list = [&](auto& xs)
		{
			append(xs.f_size(), [&](size_t i)
			{
				return t_pvalue(xs[i]);
			});
		};
		if (f_is<t_tuple>(a_headers)) {
			list(a_headers->template f_as<t_tuple>());
		} else if (f_is<t_list>(a_headers)) {
			list(a_headers->template f_as<t_list>());
		} else {
			static size_t index;
			auto size = a_headers.f_invoke(f_global()->f_symbol_size(), index);
			f_check<size_t>(size, L"size");
			append(f_as<size_t>(size), [&](size_t i)
			{
				return a_headers.f_get_at(i);
			});
		}
		a_self.f_option(CURLOPT_HTTPHEADER, headers);
	}>())
	(L"timeout"sv, t_member<void(*)(t_request&, long), [](auto a_self, auto a_seconds)
	{
		a_self.f_option(CURLOPT_TIMEOUT, a_seconds);
	}>())
	(L"send"sv, t_member<void(t_request::*)(), &t_request::f_send>())
	.f_derive<t_request, t_proxy>();
}

t_pvalue t_type_of<xemmaix::curl::t_request>::f_do_construct(t_pvalue* a_stack, size_t a_n)
{
	return t_construct<std::wstring_view>::t_bind<xemmaix::curl::t_request>::f_do(this, a_stack, a_n);
}

}
