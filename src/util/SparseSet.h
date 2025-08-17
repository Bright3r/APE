#pragma once

#include <unordered_map>
#include <vector>

namespace APE {

template <typename T>
class SparseSet {
private:
	// TODO - upgrade sparse list to a paginated vector
	std::unordered_map<size_t, size_t> m_sparse;
	std::vector<T> m_dense;

public:
	[[nodiscard]] size_t getSize() const noexcept;

	[[nodiscard]] bool empty() const noexcept;

	void remove(size_t ID) noexcept;

	void clear() noexcept;

	[[nodiscard]] bool contains(size_t ID) const noexcept;

	T& set(size_t ID, const T& val) noexcept;

	[[nodiscard]] T& get(size_t ID) noexcept;

	[[nodiscard]] std::vector<T>& data() noexcept;
};

};	// end of namespace APE
