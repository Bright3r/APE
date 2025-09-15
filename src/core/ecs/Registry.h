#pragma once

#include "core/ecs/Pool.h"

#include <bitset>
#include <cstdint>
#include <limits>
#include <tuple>
#include <utility>
#include <vector>

namespace APE::ECS {

/*
 * Aliases + Constants
*/
constexpr int MAX_NUM_COMPONENTS = 64;

using EntityID = uint64_t;
using TypeID = size_t;

struct EntityHandle {
	EntityID id;

	EntityHandle(EntityID id = calcTombstone<EntityID>()) noexcept
		: id(id)
	{ }

	bool operator==(const EntityHandle& other) const noexcept
	{
		return id == other.id;
	}

	bool operator!=(const EntityHandle& other) const noexcept
	{
		return id != other.id;
	}
};

using EntitySet = std::vector<EntityHandle>;

/*
 * Registry
*/
class Registry {
	using Bitmask = std::bitset<MAX_NUM_COMPONENTS>;

	using IPool = PoolInterface<EntityID>;

	template <typename Component>
	using CPool = Pool<EntityID, Component>;

	struct Entity {
		EntityID id;
		Bitmask component_mask;
	};

	inline static TypeID s_type_counter = 0;
	inline static EntityID s_entity_counter = 0;

	Pool<EntityID, Entity> m_entities;
	std::unordered_map<TypeID, std::unique_ptr<IPool>> m_pools;

public:
	Registry() = default;

	Registry(const Registry& other) = delete;
	Registry& operator=(const Registry& other) = delete;

	Registry(Registry&& other) = default;
	Registry& operator=(Registry&& other) = default;


	template <typename... Components>
	class View {
		using PoolsTuple = std::tuple<CPool<Components>*...>;
		using ViewEntry = std::tuple<EntityHandle, Components&...>;

		Registry* m_registry;
		PoolsTuple m_pools;
		std::vector<EntityID> m_driver_ents;
		size_t m_size;

	public:
		View(Registry* registry, PoolsTuple pools)
			: m_registry(registry)
			, m_pools(pools)
			, m_size(m_driver_ents.size())
		{
			m_driver_ents = getMinPoolEnts(pools);
			m_size = m_driver_ents.size();
		}

		View(const View& other) = default;
		View& operator=(const View& other) = default;
		View(View&& other) = default;
		View& operator=(View&& other) = default;

		std::vector<ViewEntry> each() noexcept
		{
			std::vector<ViewEntry> res;
			for (auto tup : *this) {
				res.push_back(tup);
			}
			return res;
		}

	private:
		[[nodiscard]] static decltype(auto) 
		getMinPoolEnts(const PoolsTuple& pools) noexcept
		{
			const auto* min_ents = &std::get<0>(pools)->constEntities();
			std::apply([&](auto&... pool) {
				(([&] {
					const auto& ents = pool->constEntities();
					if (ents.size() < min_ents->size()) {
						min_ents = &ents;
					}
				}()), ...);
			}, pools);

			return *min_ents;
		}

		[[nodiscard]] bool isViewMember(EntityID id) const noexcept
		{
			return m_registry->hasAllComponents<Components...>({ id });
		}

		class Iterator {
			using Entry = ViewEntry;

			View* m_view;
			size_t m_idx;

		public:
			using value_type = Entry;
			using reference = Entry;
			using pointer = void;
			using iterator_category = std::forward_iterator_tag;

			Iterator(View* view, size_t idx)
				: m_view(view)
				, m_idx(idx)
			{

			}

			Entry operator*()
			{
				EntityID id = m_view->m_driver_ents[m_idx];
				EntityHandle ent { id };

				return std::tuple_cat(
					std::forward_as_tuple(ent),
					std::apply([&](auto*... pools) {
						return std::forward_as_tuple(
							static_cast<CPool<Components>*>(pools)->get(id)...
						);
					}, m_view->m_pools)
				);
			}

			// Prefix
			Iterator& operator++()
			{
				if (++m_idx >= m_view->m_size) {
					return *this;
				}

				EntityID id { m_view->m_driver_ents[m_idx] };
				while (m_idx < m_view->m_size && 
					!m_view->isViewMember(m_view->m_driver_ents[m_idx])) 
				{
					++m_idx;
				}
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
				return m_idx == other.m_idx && m_view == other.m_view;
			}

			bool operator!=(const Iterator& other) const
			{
				return !(*this == other);
			}
		};

	public:
		Iterator begin() noexcept
		{
			Iterator first(this, 0);
			if (m_size == 0) return first;

			EntityHandle e { m_driver_ents[0] };
			if (!m_registry->hasAllComponents<Components...>(e)) {
				++first;
			}
			return first;
		}

		Iterator end() noexcept
		{
			return Iterator(this, m_size);
		}
	};


	/*
	* Entity Creation
	*/
	[[nodiscard]] EntityHandle createEntity() noexcept
	{
		EntityID ent_id = nextEntityID();
		m_entities.emplace(ent_id, ent_id, 0x0);

		return EntityHandle { ent_id };
	}

	bool destroyEntity(EntityHandle ent) noexcept
	{
		if (!m_entities.remove(ent.id)) {
			APE_WARN("Tried to destroy untracked entity {}.", ent.id);
			return false;
		}

		for (auto& [ type_id, pool ] : m_pools) {
			pool->remove(ent.id);
		}
		return true;
	}


	/*
	* Adding Components
	*/
	template <typename Component, typename... Args>
	Component& emplaceComponent(EntityHandle ent, Args&&... args) noexcept
	{
		maskEntity<Component>(ent);

		auto& pool = getPool<Component>();
		return pool.emplace(ent.id, std::forward<Args>(args)...);
	}

	template <typename Component, typename... Args>
	void emplaceComponent(const EntitySet& ents, Args&&... args) noexcept
	{
		auto& pool = getPool<Component>();
		for (auto ent : ents) {
			maskEntity<Component>(ent);
			pool.emplace(ent.id, std::forward<Args>(args)...);
		}
	}

	template <typename Component, typename... Args>
	Component& replaceComponent(EntityHandle ent, Args&&... args) noexcept
	{
		auto& pool = getPool<Component>();
		return pool.set(ent.id, std::forward<Args>(args)...);
	}

	template <typename Component, typename... Args>
	void replaceComponent(const EntitySet& ents, Args&&... args) noexcept
	{
		auto& pool = getPool<Component>();
		for (auto ent : ents) {
			pool.emplace(ent.id, std::forward<Args>(args)...);
		}
	}

	template <typename Component, typename... Args>
	Component& emplaceOrReplaceComponent(
		EntityHandle ent,
		Args&&... args) noexcept
	{
		maskEntity<Component>(ent);

		auto& pool = getPool<Component>();
		return pool.tryEmplace(ent.id, std::forward<Args>(args)...);
	}

	template <typename Component, typename... Args>
	void emplaceOrReplaceComponent(const EntitySet& ents, Args... args) noexcept
	{
		auto& pool = getPool<Component>();
		for (auto& ent : ents) {
			maskEntity<Component>(ent);
			pool.tryEmplace(ent.id, std::forward<Args>(args)...);
		}
	}

	/*
	* Removing Components
	*/
	template <typename Component>
	bool removeComponent(EntityHandle ent) noexcept
	{
		unmaskEntity<Component>(ent);

		auto& pool = getPool<Component>();
		return pool.remove(ent.id);
	}

	template <typename Component>
	void clearComponent() noexcept
	{
		auto& pool = getPool<Component>();
		for (auto ent_id : pool.entities()) {
			unmaskEntity<Component>(EntityHandle(ent_id));
		}
		pool.clear();
	}

	/*
	* Checks on Entities
	*/
	[[nodiscard]] bool isValid(const EntityHandle& ent) const noexcept
	{
		return m_entities.contains(ent.id);
	}

	template <typename... Components>
	[[nodiscard]] bool hasAllComponents(const EntityHandle& ent) noexcept
	{
		return (hasComponent<Components>(ent) && ...);
	}

	template <typename... Components>
	[[nodiscard]] bool hasAnyComponent(const EntityHandle& ent) noexcept
	{
		return (hasComponent<Components>(ent) || ...);
	}

	template <typename Component>
	[[nodiscard]] bool hasComponent(const EntityHandle& ent) noexcept
	{
		if (!isValid(ent)) {
			return false;
		}

		auto& real_ent = m_entities.get(ent.id);
		return (typeBitmask<Component>() & real_ent.component_mask) != 0;
	}

	template <typename Component>
	[[nodiscard]] bool hasComponent() const noexcept
	{
		TypeID type_id = typeID<Component>();
		return m_pools.contains(type_id);
	}

	[[nodiscard]] size_t numComponents() const noexcept
	{
		return s_type_counter;
	}

	[[nodiscard]] size_t numEntities() const noexcept
	{
		return m_entities.size();
	}

	[[nodiscard]] EntityHandle tombstone() const noexcept
	{
		return { m_entities.tombstone() };
	}


	/*
	* Retrieving Entities/Components
	*/
	template <typename... Components>
	[[nodiscard]] View<Components...> view() noexcept
	{
		auto pools = std::make_tuple(
			&getPool<Components>()...
		);
		return View<Components...>(this, pools);
	}

	template <typename... Components>
	[[nodiscard]] EntitySet entitySet() noexcept
	{
		EntitySet ents;
		for (auto ent_id : m_entities.entities()) {
			EntityHandle ent { ent_id };
			if (hasAllComponents<Components...>(ent)) {
				ents.push_back(ent);
			}
		}
		return ents;
	}

	[[nodiscard]] std::vector<EntityHandle> entities() const noexcept
	{
		std::vector<EntityID> ids = m_entities.entities();
		std::vector<EntityHandle> res;
		for (auto id : ids) {
			res.emplace_back(id);
		}
		return res;
	}

	template <typename Component>
	[[nodiscard]] CPool<Component>& getPool() noexcept
	{
		TypeID type_id = typeID<Component>();
		if (!m_pools.contains(type_id)) {
			m_pools[type_id] = std::make_unique<CPool<Component>>();
		}
		return *static_cast<CPool<Component>*>(m_pools[type_id].get());
	}

	template <typename Component>
	[[nodiscard]] const CPool<Component>& getPool() const noexcept
	{
		TypeID type_id = typeID<Component>();
		APE_CHECK((m_pools.contains(type_id)),
			"Registry::getPool() Failed: const CPool does not exist."
		);
		return *static_cast<CPool<Component>*>(m_pools.at(type_id).get());
	}

	template <typename Component>
	[[nodiscard]] decltype(auto) getComponent(const EntityHandle& ent) noexcept
	{
		APE_CHECK(hasComponent<Component>(ent),
			"Cannot get component that an entity does not have."
		);

		auto& pool = getPool<Component>();
		return pool.get(ent.id);
	}

	template <typename... Components>
	[[nodiscard]] decltype(auto) getComponents(const EntityHandle& ent) noexcept
	{
		return std::tie(getComponent<Components>(ent)...);
	}


	/*
	* Potential Future Additions
	*/

	/*
	* Event Listeners
	*/

	/*
	* In-place Sorting
	*/


private:
	[[nodiscard]] static EntityID nextEntityID() noexcept
	{
		return s_entity_counter++;
	}

	[[nodiscard]] static Bitmask nextTypeBitmask() noexcept
	{
		static Bitmask curr_mask = 0x1;

		Bitmask res = curr_mask;
		curr_mask <<= 1;
		return res;
	}

	template <typename Component>
	[[nodiscard]] static TypeID typeID() noexcept
	{
		static const TypeID id = s_type_counter++;
		return id;
	}

	template <typename Component>
	[[nodiscard]] static Bitmask typeBitmask() noexcept
	{
		static const Bitmask mask = nextTypeBitmask();
		return mask;
	}

	template <typename Component>
	void maskEntity(const EntityHandle& ent) noexcept
	{
		auto& real_ent = m_entities.get(ent.id);
		real_ent.component_mask |= typeBitmask<Component>();
	}

	template <typename Component>
	void unmaskEntity(const EntityHandle& ent) noexcept
	{
		auto& real_ent = m_entities.get(ent.id);
		real_ent.component_mask &= ~typeBitmask<Component>();
	}
};

};	// end of namespace

