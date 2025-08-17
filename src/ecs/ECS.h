#pragma once

#include <bitset>
#include <cstdint>
#include <limits>
#include <vector>

namespace APE {
namespace ECS {

constexpr int MAX_NUM_COMPONENTS = 64;

struct Entity {
	uint64_t ID;
	std::bitset<MAX_NUM_COMPONENTS> component_mask;
};


class ECS {
public:
	using View = std::vector<Entity*>;
	static constexpr uint64_t tombstone = std::numeric_limits<uint64_t>::max();

private:
	uint64_t m_curr_ID = 0;

public:
	/*
	* Entity Creation
	*/
	[[nodiscard]] Entity& createEntity() noexcept;

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
	// try to remove component from ent if it exists
	template <typename Component>
	bool removeComponent(Entity& ent) noexcept;

	template <typename Component>
	void clearComponent() noexcept;

	/*
	* Checks on Entities
	*/
	[[nodiscard]] bool isValid(const Entity& ent) const noexcept;

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
};

};	// end of namespace ECS
};	// end of namespace APE
