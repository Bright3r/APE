#pragma once

#include "util/Logger.h"

#include <algorithm>
#include <functional>
#include <limits>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace APE::ECS {

template <typename EntityID>
struct PoolInterface {
	virtual ~PoolInterface() = default;

	virtual bool remove(EntityID id) noexcept = 0;
};

template <typename EntityID, typename T>
class Pool : public PoolInterface<EntityID> {
private:
	// TODO - upgrade sparse list to a paginated vector
	// Map entity id to component in dense array
	std::unordered_map<EntityID, size_t> m_sparse;

	// Contains tightly-packed POD components
	std::vector<T> m_dense;

	// Maps dense component index to its EntityID
	std::vector<EntityID> m_denseToID;

	// Reserved id representing a null entity
	EntityID m_tombstone;

public:
	Pool() noexcept
	{
		static_assert(
			std::is_integral_v<EntityID>, 
			"Pool only supports integral EntityID types."
		);
		m_tombstone = std::numeric_limits<EntityID>::max();
	}

	[[nodiscard]] EntityID tombstone() const noexcept
	{
		return m_tombstone;
	}

	[[nodiscard]] size_t size() const noexcept
	{
		return m_dense.size();
	}

	[[nodiscard]] std::vector<EntityID> entities() const noexcept
	{
		return m_denseToID;
	}

	[[nodiscard]] bool empty() const noexcept
	{
		return m_dense.empty();
	}

	void clear() noexcept
	{
		m_sparse.clear();
		m_dense.clear();
		m_denseToID.clear();
	}

	[[nodiscard]] bool contains(EntityID id) const noexcept
	{
		return (id != m_tombstone) && 
			(m_sparse.find(id) != m_sparse.end());
	}

	[[nodiscard]] bool remove(EntityID id) noexcept override
	{
		if (empty()) {
			APE_ERROR(
				"Pool::remove() Failed: cannot remove entity {} because set is empty.",
				id
			);
			return false;
		}

		if (!isValidID(id)) {
			APE_ERROR(
				"Pool::remove() Failed: entity {} is not in the set.",
				id
			);
			return false;
		}

		// Swap removal entity's component to back of dense array
		EntityID remove_id = id;
		EntityID swap_id = getEntityID(m_dense.size() - 1);
		size_t remove_idx = getDenseIdx(remove_id);
		size_t swap_idx = getDenseIdx(swap_id);

		std::swap(
			m_dense[swap_idx],
	    		m_dense[remove_idx]
		);

		std::swap(
			m_denseToID[swap_idx],
			m_denseToID[remove_idx]
		);

		// Pop off removed component from back of dense array
		m_dense.pop_back();
		m_denseToID.pop_back();

		// Update sparse list to reflect updated dense array
		m_sparse[swap_id] = remove_idx;
		m_sparse.erase(remove_id);

		return true;
	}

	template <typename... Args>
	T& emplace(EntityID id, Args&&... args) noexcept
	{
		APE_CHECK(!contains(id),
			"Pool::emplace() Failed: set already contains entity {}'s component. Use set instead to replace component data.",
			id
		);
	
		m_dense.emplace_back(std::forward<Args>(args)...);
		m_denseToID.emplace_back(id);
		m_sparse[id] = m_dense.size() - 1;

		return m_dense.back();
	}

	template <typename... Args>
	T& tryEmplace(EntityID id, Args&&... args) noexcept
	{
		if (contains(id)) {
			return set(id, std::forward<Args>(args)...);
		}

		return emplace(id, std::forward<Args>(args)...);
	}

	template <typename... Args>
	T& set(EntityID id, Args&&... args) noexcept
	{
		APE_CHECK(contains(id),
			"Pool::set() Failed: set does not contain entity {}'s component.",
			id
		);
	
		size_t dense_idx = getDenseIdx(id);
		m_dense[dense_idx] = T(args...);

		return m_dense[dense_idx];
	}

	[[nodiscard]] T& get(EntityID id) noexcept
	{
		APE_CHECK(contains(id),
			"Pool::get() Failed: set does not contain entity {}'s component.",
			id
		);

		return m_dense[getDenseIdx(id)];
	}

	[[nodiscard]] const T& get(EntityID id) const noexcept
	{
		APE_CHECK(contains(id),
			"Pool::get() Failed: set does not contain entity {}'s component.",
			id
		);

		return m_dense[getDenseIdx(id)];
	}

	void forEach(std::function<void(T&)> fn)
	{
		std::for_each(m_dense.begin(), m_dense.end(), fn);
	}

private:
	[[nodiscard]] size_t getDenseIdx(EntityID id) const noexcept
	{
		APE_CHECK(isValidID(id),
			"Pool::getDenseIdx() Failed: invalid entity id of {}",
	    		id
		);

		return m_sparse.at(id);
	}

	[[nodiscard]] EntityID getEntityID(size_t dense_idx) const noexcept
	{
		APE_CHECK((dense_idx < m_dense.size()),
			"Pool::getEntityID() Failed: dense index out of bounds."
		);

		return m_denseToID.at(dense_idx);
	}

	[[nodiscard]] bool isValidID(EntityID id) const noexcept
	{
		if (id == m_tombstone) {
			APE_ERROR(
				"Pool::isValidID() Failed: Cannot remove tombstone."
			);
			return false;
		}

		if (!contains(id)) {
			APE_ERROR(
				"Pool::isValidID() Failed: entity {} is not in the set.",
				id
			);
			return false;
		}

		return true;
	}

public:
	struct Entry {
		EntityID id;
		T& component;
	};

	/*
	* Custom Iterator that returns component data with corresponding EntityID
	* Iterates in reverse order under the hood so that Iterators are not
	* invalidated from deletions
	*/
	class Iterator {
	private:
		Pool* m_set;
		size_t m_idx;

	public:
		using value_type = Entry;
		using reference = Entry;
		using pointer = void;
		using iterator_category = std::forward_iterator_tag;

		Iterator(Pool* set, size_t idx) noexcept
			: m_set(set)
			, m_idx(idx)
		{

		}

		Entry operator*() const 
		{
			return {
				m_set->m_denseToID[m_idx-1],
				m_set->m_dense[m_idx-1]
			};
		}

		// Prefix
		Iterator& operator++() 
		{
			--m_idx;
			return *this;
		}

		// Postfix
		Iterator operator++(int) 
		{
			Iterator tmp = *this;
			++(*this);
			return tmp;
		}

		bool operator==(const Iterator& other) const 
		{
			return m_idx == other.m_idx && m_set == other.m_set;
		}

		bool operator!=(const Iterator& other) const 
		{
			return !(*this == other);
		}
	};


	class ConstIterator {
	private:
		const Pool* m_set;
		size_t m_idx;

	public:
		using value_type = Entry;
		using reference = Entry;
		using pointer = void;
		using iterator_category = std::forward_iterator_tag;

		ConstIterator(const Pool* set, size_t idx) noexcept
			: m_set(set)
			, m_idx(idx)
		{

		}

		Entry operator*() const 
		{
			return {
				m_set->m_denseToID[m_idx-1],
				m_set->m_dense[m_idx-1]
			};
		}

		// Prefix
		ConstIterator& operator++() 
		{
			--m_idx;
			return *this;
		}

		// Postfix
		ConstIterator operator++(int) 
		{
			ConstIterator tmp = *this;
			++(*this);
			return tmp;
		}

		bool operator==(const ConstIterator& other) const 
		{
			return m_idx == other.m_idx && m_set == other.m_set;
		}

		bool operator!=(const ConstIterator& other) const 
		{
			return !(*this == other);
		}
	};


	[[nodiscard]] Iterator begin() noexcept
	{
		return Iterator(this, m_dense.size());
	}

	[[nodiscard]] Iterator end() noexcept
	{
		return Iterator(this, 0);
	}

	[[nodiscard]] ConstIterator begin() const noexcept
	{
		return ConstIterator(this, m_dense.size());
	}

	[[nodiscard]] ConstIterator end() const noexcept
	{
		return ConstIterator(this, 0);
	}
};

};	// end of namespace

