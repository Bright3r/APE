#pragma once

#include "util/SparseSet.h"

#include <bitset>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

namespace APE {
namespace ECS {

/*
 * Aliases + Constants
*/
using EntityID = uint64_t;
using TypeID = size_t;

constexpr int MAX_NUM_COMPONENTS = 64;
using Bitmask = std::bitset<MAX_NUM_COMPONENTS>;

using PoolInterface = SparseSetInterface;

template <typename Component>
using Pool = SparseSet<EntityID, Component>;

/*
 * Entity
*/
struct Entity {
	EntityID id;
	Bitmask component_mask;
};



/*
 * ECS
*/
class ECS {
public:
	using View = std::vector<Entity*>;

private:
	inline static TypeID s_type_counter = 0;
	inline static EntityID s_entity_counter = 0;

	SparseSet<EntityID, Entity> m_entities;
	std::unordered_map<TypeID, std::unique_ptr<PoolInterface>> m_pools;

public:
	/*
	* Entity Creation
	*/
	[[nodiscard]] Entity& createEntity() noexcept
	{
		EntityID ent_id = nextEntityID();
		m_entities.emplace(ent_id, ent_id, 0x0);

		return m_entities.get(ent_id);
	}

	void destroyEntity(Entity& ent) noexcept;

	/*
	* Adding Components
	*/
	template <typename Component, typename... Args>
	Component& emplaceComponent(Entity& ent, Args&&... args) noexcept
	{
		maskEntity<Component>(ent);

		auto pool = getPool<Component>();
		return pool.emplace(ent.id, std::forward<Args>(args)...);
	}

	template <typename Component, typename... Args>
	void emplaceComponent(const View& ents, Args&&... args) noexcept
	{
		auto pool = getPool<Component>();
		for (Entity* ent : ents) {
			maskEntity<Component>(*ent);
			pool.emplace(ent->id, std::forward<Args>(args)...);
		}
	}

	template <typename Component>
	Component& replaceComponent(Entity& ent, Component&& comp) noexcept
	{
		auto pool = getPool<Component>();
		return pool.set(ent.id, std::forward(comp));
	}

	template <typename Component>
	void replaceComponent(const View& ents, Component&& comp) noexcept
	{
		auto pool = getPool<Component>();
		for (Entity* ent : ents) {
			pool.emplace(ent->id, std::forward<Component>(comp));
		}
	}

	template <typename Component, typename... Args>
	Component& emplaceOrReplaceComponent(Entity& ent, Args&&... args) noexcept
	{
		maskEntity<Component>(ent);

		auto pool = getPool<Component>();
		pool.tryEmplace(ent.id, std::forward<Args>(args)...);
	}

	template <typename Component, typename... Args>
	void emplaceOrReplaceComponent(const View& ents, Args... args) noexcept
	{
		auto pool = getPool<Component>();
		for (Entity* ent : ents) {
			maskEntity<Component>(*ent);
			pool.tryEmplace(ent->id, std::forward<Args>(args)...);
		}
	}

	/*
	* Removing Components
	*/
	template <typename Component>
	bool removeComponent(Entity& ent) noexcept
	{
		unmaskEntity<Component>(ent);

		auto pool = getPool<Component>();
		return pool.remove(ent.id);
	}

	template <typename Component>
	void clearComponent() noexcept
	{
		auto pool = getPool<Component>();
		for (auto [ ent_id, comp ] : pool) {
			unmaskEntity<Component>(ent_id);
		}
		m_pools.erase(typeID<Component>());
	}

	/*
	* Checks on Entities
	*/
	[[nodiscard]] bool isValid(const Entity& ent) const noexcept
	{
		return m_entities.contains(ent.id);
	}

	template <typename... Components>
	[[nodiscard]] bool hasAllComponents(const Entity& ent) const noexcept;

	template <typename... Components>
	[[nodiscard]] bool hasAnyComponent(const Entity& ent) const noexcept;

	/*
	* Retrieving Entities/Components
	*/
	template <typename... Components>
	[[nodiscard]] View getView() const noexcept;

	template <typename... Components>
	[[nodiscard]] decltype(auto) getComponents(Entity& ent) noexcept;


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
	inline void maskEntity(Entity& ent) noexcept
	{
		ent.component_mask |= typeBitmask<Component>();
	}

	template <typename Component>
	inline void unmaskEntity(Entity& ent) noexcept
	{
		ent.component_mask ^= typeBitmask<Component>();
	}

	template <typename Component>
	[[nodiscard]] Pool<Component>& getPool() noexcept
	{
		TypeID type_id = typeID<Component>();
		if (!m_pools.contains(type_id)) {
			m_pools[type_id] = std::make_unique<Pool<Component>>();
		}
		return *static_cast<Pool<Component>*>(m_pools[type_id].get());
	}
};

};	// end of namespace ECS
};	// end of namespace APE

