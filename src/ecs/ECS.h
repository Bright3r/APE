#pragma once

#include <bitset>
#include <cstdint>
#include <unordered_map>
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

	std::unordered_map<EntityID, Entity> m_entities;

public:
	/*
	* Entity Creation
	*/
	[[nodiscard]] Entity& createEntity() noexcept
	{
		EntityID ent_id = nextEntityID();
		m_entities[ent_id] = {
			.id = ent_id,
			.component_mask = 0x0,
		};

		return m_entities[ent_id];
	}

	void destroyEntity(Entity& ent) noexcept;

	/*
	* Adding Components
	*/
	template <typename Component>
	Component& addComponent(Entity& ent) noexcept;

	template <typename Component>
	void addComponent(const View& ents) noexcept;

	template <typename Component>
	Component& addComponent(Entity& ent, const Component& comp) noexcept;

	template <typename Component>
	void addComponent(const View& ents, const Component& comp) noexcept;

	template <typename Component, typename... Args>
	Component& emplaceComponent(Entity& ent, Args... args) noexcept;

	template <typename Component, typename... Args>
	void emplaceComponent(const View& ents, Args... args) noexcept;

	template <typename Component, typename... Args>
	Component& replaceComponent(Entity& ent, Args... args) noexcept;

	template <typename Component, typename... Args>
	void replaceComponent(const View& ents, Args... args) noexcept;

	template <typename Component, typename... Args>
	Component& emplaceOrReplaceComponent(Entity& ent, Args... args) noexcept;

	template <typename Component, typename... Args>
	void emplaceOrReplaceComponent(const View& ents, Args... args) noexcept;

	/*
	* Removing Components
	*/
	template <typename Component>
	bool removeComponent(Entity& ent) noexcept;

	template <typename Component>
	bool tryRemoveComponent(Entity& ent) noexcept;

	template <typename Component>
	void clearComponent() noexcept;

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
};

};	// end of namespace ECS
};	// end of namespace APE

