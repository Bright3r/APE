#pragma once

#include "ecs/Pool.h"

#include <bitset>
#include <cstdint>
#include <tuple>
#include <utility>
#include <vector>

namespace APE::ECS {

/*
 * Aliases + Constants
*/
using EntityID = uint64_t;
using TypeID = size_t;

constexpr int MAX_NUM_COMPONENTS = 64;
using Bitmask = std::bitset<MAX_NUM_COMPONENTS>;

using IPool = PoolInterface<EntityID>;

template <typename Component>
using CPool = Pool<EntityID, Component>;

struct EntityHandle {
	EntityID id;
};
using View = std::vector<EntityHandle>;

/*
 * Registry
*/
class Registry {
private:
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
	void emplaceComponent(const View& ents, Args&&... args) noexcept
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
	void replaceComponent(const View& ents, Args&&... args) noexcept
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
	void emplaceOrReplaceComponent(const View& ents, Args... args) noexcept
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
		for (auto [ ent_id, comp ] : pool) {
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
		auto& real_ent = m_entities.get(ent.id);
		return (typeBitmask<Component>() & real_ent.component_mask) != 0;
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
	[[nodiscard]] View view() noexcept
	{
		View res;
		for (auto [ id, ent ] : m_entities) {
			if (hasAllComponents<Components...>(EntityHandle(id))) {
				res.emplace_back(EntityHandle(id));
			}
		}
		return res;
	}

	template <typename Component>
	[[nodiscard]] Component& getComponent(const EntityHandle& ent) noexcept
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

	template <typename Component>
	[[nodiscard]] CPool<Component>& getPool() noexcept
	{
		TypeID type_id = typeID<Component>();
		if (!m_pools.contains(type_id)) {
			m_pools[type_id] = std::make_unique<CPool<Component>>();
		}
		return *static_cast<CPool<Component>*>(m_pools[type_id].get());
	}
};

};	// end of namespace

