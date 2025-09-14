#pragma once

#include <nfd.hpp>

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace APE::Files {

using Filters = std::vector<std::pair<std::string, std::string>>;

enum class Status {
	Sucess = 0,
	Canceled,
	Failure
};

[[nodiscard]] inline Status openDialog(
	std::filesystem::path& out_path,
	Filters filters = {}) noexcept
{
	static bool b_initialized = false;
	if (!b_initialized) {
		NFD_Init();
		b_initialized = true;
	}

	std::vector<nfdu8filteritem_t> nfd_filters;
	for (auto& filter : filters) {
		nfdu8filteritem_t nfd_filter { 
			filter.first.c_str(),
			filter.second.c_str() 
		};
		nfd_filters.emplace_back(nfd_filter);
	}

	nfdopendialogu8args_t args = { 0 };
	args.filterList = nfd_filters.data();
	args.filterCount = nfd_filters.size();

	nfdu8char_t* nfd_path;
	nfdresult_t res = NFD_OpenDialogU8_With(&nfd_path, &args);
	Status status = Status::Failure;
	if (res == NFD_OKAY) {
		out_path = nfd_path;
		NFD_FreePathU8(nfd_path);
		status = Status::Sucess;
	}
	else if (res == NFD_CANCEL) {
		status = Status::Canceled;
	}

	return status;
}

};	// end of namespace

